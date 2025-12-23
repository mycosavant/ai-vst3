import torch
import numpy as np
import tempfile
import os
import time
import random
import gc
import librosa
import soundfile as sf
from diffusers import BitsAndBytesConfig as DiffusersBitsAndBytesConfig, StableAudioDiTModel, StableAudioPipeline
from transformers import BitsAndBytesConfig as TransformersBitsAndBytesConfig, T5EncoderModel


class MusicGenerator:
    def __init__(self, model_id="stabilityai/stable-audio-open-1.0"):
        self.model_id = model_id
        self.pipeline = None
        self.sample_rate = 44100
        self.default_duration = 6
        self.sample_cache = {}

    def init_model(self):
        print(f"⚡ Initializing Stable Audio Model with quantization...")

        device = "cuda" if torch.cuda.is_available() else "cpu"
        print(f"ℹ️  Using device: {device}")

        if device == "cuda" and self.model_id == "stabilityai/stable-audio-open-1.0":
            print("🎯 Loading with 8-bit quantization...")
            
            text_encoder_quant_config = TransformersBitsAndBytesConfig(load_in_8bit=True)
            text_encoder_8bit = T5EncoderModel.from_pretrained(
                self.model_id,
                subfolder="text_encoder",
                quantization_config=text_encoder_quant_config,
                torch_dtype=torch.float16,
            )
            
            transformer_quant_config = DiffusersBitsAndBytesConfig(load_in_8bit=True)
            transformer_8bit = StableAudioDiTModel.from_pretrained(
                self.model_id,
                subfolder="transformer",
                quantization_config=transformer_quant_config,
                torch_dtype=torch.float16,
            )
            
            self.pipeline = StableAudioPipeline.from_pretrained(
                self.model_id,
                text_encoder=text_encoder_8bit,
                transformer=transformer_8bit,
                torch_dtype=torch.float16,
                device_map="balanced",
            )
        else:
            print("📦 Loading without quantization...")
            self.pipeline = StableAudioPipeline.from_pretrained(
                self.model_id,
                torch_dtype=torch.float16 if device == "cuda" else torch.float32,
            )
            self.pipeline = self.pipeline.to(device)

        self.sample_rate = self.pipeline.vae.sampling_rate
        self.device = device

        print(f"✅ Stable Audio initialized (sample rate: {self.sample_rate}Hz)!")

    def destroy_model(self):
        self.pipeline = None
        if torch.cuda.is_available():
            torch.cuda.empty_cache()
        gc.collect()

    def generate_sample(
        self,
        musicgen_prompt,
        tempo,
        sample_type="custom",
        generation_duration=6,
        sample_rate=48000,
    ):
        try:
            print(f"🔮 Direct generation with prompt: '{musicgen_prompt}'")

            num_inference_steps = 50
            cfg_scale = 7.0
            
            if "small" in self.model_id.lower():
                num_inference_steps = 8
                cfg_scale = 1.0

            seed_value = random.randint(0, 2**31 - 1)
            generator = torch.Generator(device=self.device).manual_seed(seed_value)

            print(f"⚙️  Stable Audio: steps={num_inference_steps}, cfg_scale={cfg_scale}")

            start_gen = time.time()
            
            result = self.pipeline(
                musicgen_prompt,
                negative_prompt="Low quality, distorted, noise",
                num_inference_steps=num_inference_steps,
                audio_end_in_s=generation_duration,
                num_waveforms_per_prompt=1,
                generator=generator,
                guidance_scale=cfg_scale,
            )

            print(f"✅ Diffusion steps complete in {time.time() - start_gen:.2f}s!")

            start_post = time.time()

            output = result.audios[0].T.float().cpu().numpy()
            
            if len(output.shape) > 1 and output.shape[0] > 1:
                sample_audio = output[0]
            else:
                sample_audio = output.flatten()

            print(f"⏱️  Total post-processing: {time.time() - start_post:.2f}s")
            print(f"✅ Generation complete!")

            sample_info = {
                "type": sample_type,
                "tempo": tempo,
                "prompt": musicgen_prompt,
            }

            return sample_audio, sample_info

        except Exception as e:
            print(f"❌ Generation error: {str(e)}")
            silence = np.zeros(sample_rate * 4)
            error_info = {"type": sample_type, "tempo": tempo, "error": str(e)}
            return silence, error_info

    def save_sample(self, sample_audio, filename, sample_rate=48000):
        try:
            if filename.endswith(".wav"):
                path = filename
            else:
                temp_dir = tempfile.gettempdir()
                path = os.path.join(temp_dir, filename)
            
            if not isinstance(sample_audio, np.ndarray):
                sample_audio = np.array(sample_audio)
            
            if self.sample_rate != sample_rate:
                print(f"🔄 Resampling {self.sample_rate}Hz → {sample_rate}Hz")
                sample_audio = librosa.resample(
                    sample_audio, orig_sr=self.sample_rate, target_sr=sample_rate
                )
                save_sample_rate = sample_rate
            else:
                save_sample_rate = self.sample_rate

            max_val = np.max(np.abs(sample_audio))
            if max_val > 0:
                sample_audio = sample_audio / max_val * 0.9

            sf.write(path, sample_audio, save_sample_rate)

            return path
        except Exception as e:
            print(f"❌ Error saving sample: {e}")
            return None