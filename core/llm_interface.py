import json
import time
import gc
import threading
from datetime import datetime, timedelta
from apscheduler.schedulers.background import BackgroundScheduler
from llama_cpp import Llama
import torch


class DJAILL:
    def __init__(self, model_path, config=None):
        self.model_path = model_path
        self.session_state = config or {}
        self.conversations = {}
        self.scheduler = BackgroundScheduler()
        self.scheduler.add_job(
            func=self.scheduled_cleanup, trigger="interval", minutes=60
        )
        self.conversations_lock = threading.Lock()

    def destroy_model(self):
        if hasattr(self, "model"):
            try:
                del self.model
                if torch.cuda.is_available():
                    torch.cuda.empty_cache()
                gc.collect()
                print("🧹 Model destroyed")
            except Exception as e:
                print(f"❌ Error destroying model: {e}")

    def init_model(self):
        print(f"⚡ Initializing LLM model from {self.model_path}...")
        self.model = Llama(
            model_path=self.model_path,
            n_ctx=4096,
            n_gpu_layers=-1,
            n_threads=4,
            verbose=False,
        )
        print("✅ New LLM model initialized")

    def scheduled_cleanup(self):
        if not self.conversations:
            print("🧹 Cleanup: No conversations to clean")
            return

        with self.conversations_lock:
            cutoff_time = datetime.now() - timedelta(seconds=3600)
            initial_count = len(self.conversations)

            self.conversations = {
                user_id: conv
                for user_id, conv in self.conversations.items()
                if not (
                    isinstance(conv, dict)
                    and conv.get("last_message_timestamp", datetime.now()) < cutoff_time
                )
            }

            final_count = len(self.conversations)
            cleaned_count = initial_count - final_count

            if cleaned_count > 0:
                print(
                    f"🧹 Cleanup: Removed {cleaned_count} old conversation(s), {final_count} remaining"
                )
            else:
                print(f"🧹 Cleanup: No old conversations found, {final_count} active")

    def init_user_conversation_history(self, user_id):
        if user_id not in self.conversations:
            self.conversations[user_id] = {
                "conversation_history": [
                    {"role": "system", "content": self.get_system_prompt()}
                ],
                "last_message_timestamp": datetime.now(),
            }

    def get_next_decision(self):
        current_time = time.time()
        if self.session_state.get("last_action_time", 0) > 0:
            elapsed = current_time - self.session_state["last_action_time"]
            self.session_state["session_duration"] = (
                self.session_state.get("session_duration", 0) + elapsed
            )
        self.session_state["last_action_time"] = current_time

        user_prompt = self._build_prompt()
        user_id = self.session_state["user_id"]

        with self.conversations_lock:
            self.init_user_conversation_history(user_id)
            self.conversations[user_id]["conversation_history"].append(
                {"role": "user", "content": user_prompt}
            )

        print(
            f"🧠 AI-DJ generation with {len(self.conversations[user_id]['conversation_history'])} history messages..."
        )

        response = self.model.create_chat_completion(
            messages=self.conversations[user_id]["conversation_history"],
            response_format={
                "type": "json_object",
                "schema": {
                    "type": "object",
                    "properties": {
                        "action_type": {"type": "string", "enum": ["generate_sample"]},
                        "parameters": {
                            "type": "object",
                            "properties": {
                                "sample_details": {
                                    "type": "object",
                                    "properties": {
                                        "musicgen_prompt": {"type": "string"},
                                        "key": {"type": "string"},
                                    },
                                    "required": ["musicgen_prompt", "key"],
                                }
                            },
                            "required": ["sample_details"],
                        },
                        "reasoning": {"type": "string"},
                    },
                    "required": ["action_type", "parameters", "reasoning"],
                },
            },
            temperature=0.7,
        )

        print("✅ Generation complete!")

        try:
            decision = json.loads(response["choices"][0]["message"]["content"])

        except (json.JSONDecodeError, KeyError) as e:
            print(f"❌ Error parsing response: {e}")

        self.conversations[user_id]["conversation_history"].append(
            {"role": "assistant", "content": json.dumps(decision)}
        )
        self.conversations[user_id]["last_message_timestamp"] = datetime.now()

        if len(self.conversations[user_id]["conversation_history"]) > 9:
            del self.conversations[user_id]["conversation_history"][1:3]

        return decision

    def _build_prompt(self):
        user_prompt = self.session_state.get("user_prompt", "")
        current_tempo = self.session_state.get("current_tempo", 126)
        current_key = self.session_state.get("current_key", "C minor")

        return f"""⚠️ NEW USER PROMPT ⚠️
Keywords: {user_prompt}

Context:
- Tempo: {current_tempo} BPM
- Key: {current_key}

IMPORTANT: This new prompt has PRIORITY. If it's different from your previous generation, ABANDON the previous style completely and focus on this new prompt."""

    def get_system_prompt(self) -> str:
        return """You are a smart music sample generator. The user provides you with keywords, you generate coherent prompt.


PRIORITY RULES:
1. 🔥 IF the user requests a specific style/genre → IGNORE the history and generate exactly what they ask for
2. 📝 IF it's a vague or similar request → You can consider the history for variety
3. 🎯 ALWAYS respect keywords User's exact

TECHNICAL RULES:
- Create a consistent and accurate MusicGen prompt
- For the key: use the one provided

EXAMPLES:
User: "deep techno rhythm kick hardcore" → musicgen_prompt: "deep techno kick drum, hardcore rhythm, driving 4/4 beat, industrial"
User: "ambient space" → musicgen_prompt: "ambient atmospheric space soundscape, ethereal pads"
User: "jazzy piano" → musicgen_prompt: "jazz piano, smooth chords, melodic improvisation"""

    def reset_conversation(self, user_id):
        with self.conversations_lock:
            if user_id in self.conversations:
                self.conversations[user_id] = {
                    "conversation_history": [
                        {"role": "system", "content": self.get_system_prompt()}
                    ],
                    "last_message_timestamp": datetime.now(),
                }
                print(f"🔄 Conversation history reset for user {user_id}")
