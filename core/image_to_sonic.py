import base64
import gc
import json
import torch
from llama_cpp import Llama
from llama_cpp.llama_chat_format import MiniCPMv26ChatHandler


MUSICAL_VISION_SYSTEM_PROMPT = """You are a synesthetic AI that translates visual scenes into detailed musical and sonic descriptions optimized for audio generation.

Your task is to analyze the visual description and extract sonic/musical characteristics that match the provided tempo and key.

MUSICAL ELEMENTS TO IDENTIFY:
- Rhythm: steady, syncopated, flowing, staccato, irregular, polyrhythmic (must match provided BPM)
- Melody: ascending, descending, repetitive, chaotic, harmonic, minimal (must fit provided key)
- Harmony: consonant, dissonant, chord progressions (within provided key)
- Dynamics: quiet (pp), soft (p), moderate (mf), loud (f), very loud (ff), dynamic contrasts
- Timbre: bright, dark, warm, cold, metallic, organic, synthetic, distorted

SONIC QUALITIES TO EXTRACT:
- Texture: smooth, rough, dense, sparse, layered, granular
- Space: intimate, vast, reverberant, dry, distant, close, cavernous
- Movement: static, flowing, pulsing, swirling, drifting, chaotic
- Density: minimal, moderate, complex, overwhelming, sparse-to-dense

INSTRUMENTATION ANALYSIS:
- Primary instruments or sound sources matching the scene
- Electronic vs acoustic balance
- Percussive vs melodic emphasis
- Ambient vs rhythmic components
- Synthesis types (subtractive, FM, granular, wavetable, etc.)

EMOTIONAL MAPPING:
- Dominant mood: peaceful, tense, joyful, melancholic, mysterious, energetic, contemplative
- Energy level: 1-10 scale (1=calm, 10=chaotic)
- Sonic color: bright/dark spectrum, warm/cold spectrum

OUTPUT FORMAT (MANDATORY JSON):
You MUST respond with ONLY valid JSON in this exact structure:

{
    "action_type": "generate_sample",
    "parameters": {
        "sample_details": {
            "musicgen_prompt": "[Detailed prompt for MusicGen: genre, instruments, mood, texture, dynamics - max 200 words, comma-separated descriptors. DO NOT include tempo or key as they are provided separately]",
            "key": "[Use the provided key exactly as given]",
            "bpm": [Use the provided BPM value],
            "duration": [Suggested duration in seconds, typically 10-30]
        },
        "sonic_analysis": {
            "atmosphere": "[1-2 sentence overall sonic description]",
            "primary_elements": ["element1", "element2", "element3"],
            "instrumentation": ["instrument1", "instrument2", "instrument3"],
            "mood": "[dominant emotional quality]",
            "energy_level": [1-10],
            "texture": "[sonic texture descriptor]",
            "space": "[spatial quality]"
        }
    },
    "reasoning": "[2-3 sentences explaining your sonic translation choices and how visual elements map to audio characteristics]"
}

CRITICAL RULES:
1. Output ONLY valid JSON - no markdown, no code blocks, no explanations outside JSON
2. The musicgen_prompt must be detailed and specific but MUST NOT include tempo/BPM or key information (they are provided separately)
3. Use the provided BPM and key values in your response
4. Use concrete musical terms, not visual descriptions
5. Focus on what can be HEARD, not seen
6. Ensure rhythm and melodic suggestions align with the provided tempo and key
7. All JSON fields must be properly formatted with correct types

Example musicgen_prompt (WITHOUT tempo/key):
"ambient electronic soundscape, warm analog synthesizers, gentle pads, subtle field recordings, reverberant space, peaceful and contemplative mood, sparse percussion, organic textures, gradual evolution, layered atmospheres"
"""


def generate_img_description(img_path, bpm=127, scale="C Minor", temperature=0.7):

    chat_handler = MiniCPMv26ChatHandler(clip_model_path="models/mmproj-model-f16.gguf")
    llm = Llama(
        model_path="models/ggml-model-Q4_K_M.gguf",
        chat_handler=chat_handler,
        n_ctx=4096,
    )

    user_message = f"""Translate this image into a sonic/musical description.

CONTEXT:
- Tempo: {bpm} BPM
- Key: {scale}

Your description must work within these constraints."""

    with open(img_path, "rb") as f:
        img_b64 = base64.b64encode(f.read()).decode("utf-8")

    response = llm.create_chat_completion(
        messages=[
            {"role": "system", "content": MUSICAL_VISION_SYSTEM_PROMPT},
            {
                "role": "user",
                "content": [
                    {"type": "text", "text": user_message},
                    {
                        "type": "image_url",
                        "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"},
                    },
                ],
            },
        ],
        temperature=temperature,
        max_tokens=1000,
    )

    chat_handler = None
    llm = None
    gc.collect()
    if torch.cuda.is_available():
        torch.cuda.empty_cache()

    response_text = response["choices"][0]["message"]["content"]
    response_text = response_text.strip()
    if response_text.startswith("```json"):
        response_text = response_text.split("```json")[1].split("```")[0]

    sonic_json = json.loads(response_text)
    return sonic_json


def extract_musicgen_prompt(sonic_json, bpm, scale):
    base_prompt = sonic_json["parameters"]["sample_details"]["musicgen_prompt"]
    return f"{base_prompt}, {bpm} BPM, {scale} tonality"
