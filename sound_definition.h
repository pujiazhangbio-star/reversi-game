 #ifndef SOUND_DEFINITIONS_H
 #define SOUND_DEFINITIONS_H
 #include <string>

 enum class SoundType {
	 PIECE_PLACED,
	 CAPTURE,
	 VALID_MOVE,
	 INVALID_MOVE,
	 GAME_START,
	 GAME_END,
	 PLAYER_SWITCH,
	 BUTTON_CLICK
 };

 struct SoundConfig {
	 SoundType type;
	 std::string name;
	 std::string filename;
 };

 static const SoundConfig DEFAULT_SOUNDS[] = {
	 {SoundType::PIECE_PLACED, "place_piece", "sounds/place_piece.wav"},
	 {SoundType::CAPTURE, "capture", "sounds/capture.wav"},
	 {SoundType::VALID_MOVE, "valid_move", "sounds/valid_move.wav"},
	 {SoundType::INVALID_MOVE, "invalid_move", "sounds/invalid_move.wav"},
	 {SoundType::GAME_START, "game_start", "sounds/game_start.wav"},
	 {SoundType::GAME_END, "game_end", "sounds/game_end.wav"},
	 {SoundType::PLAYER_SWITCH, "player_switch", "sounds/player_switch.wav"},
	 {SoundType::BUTTON_CLICK, "button_click", "sounds/button_click.wav"}
 };

 static const int NUM_SOUNDS = sizeof(DEFAULT_SOUNDS) / sizeof(SoundConfig);

 #endif // SOUND_DEFINITIONS_H
