// ────────────────────────────────────────────────
// main.cpp
// 프로그램 진입점
// 커맨드라인 인자로 시작 스테이지와 틱 간격을 받아
// Game 객체를 생성하고 실행
// ────────────────────────────────────────────────

#include <iostream>
#include "game.hpp"

// 사용법: ./snakegame [시작_스테이지(0~3)] [틱_배수]
// 예) ./snakegame 0 3  → 스테이지 1부터, tick=150ms
int main(int argc, char* argv[]) {
    int map_index = 0;   // 시작 스테이지 (0~3)
    int tick_ms   = 150; // 기본 틱 간격 (ms)

    if (argc > 1) {
        map_index = std::atoi(argv[1]);
        if (map_index < 0) map_index = 0;
        if (map_index > 3) map_index = 3;
    }
    if (argc > 2) {
        // 입력값에 50을 곱해 틱 간격 결정 (예: 3 → 150ms)
        tick_ms = std::atoi(argv[2]) * 50;
        if (tick_ms < 10) tick_ms = 10;
    }

    Game game(map_index, tick_ms);
    game.run();

    return 0;
}
