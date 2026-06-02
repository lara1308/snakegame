// ────────────────────────────────────────────────
// game.hpp
// 게임 루프, 스테이지 전환, 아이템·타이머 관리,
// 충돌 처리를 담당하는 Game 클래스 선언
// ────────────────────────────────────────────────

#pragma once
#include "snake.hpp"
#include "map.hpp"
#include "contents.hpp"
#include "gate.hpp"
#include <chrono>

// 방향 상수
enum Direction { UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3 };

// 스테이지별 미션 목표값 구조체
struct MissionGoal {
    int body;    // 목표 몸 길이
    int growth;  // 목표 Growth Item 획득 수
    int poison;  // 목표 Poison Item 획득 수
    int gate;    // 목표 Gate 사용 횟수
};

class Game {
public:
    // map_index: 시작 스테이지(0~3), tick_ms: 틱 간격(ms)
    explicit Game(int map_index = 0, int tick_ms = 150);
    void run(); // 게임 전체 루프

private:
    // ── 게임 상태 ──────────────────────────────
    int  current_stage;    // 현재 스테이지 (0~3)
    int  tick_ms;          // 틱 간격 (ms)
    bool is_running;       // 게임 루프 진행 여부
    bool quit_requested;   // q 키로 강제 종료 여부

    // ── 통계 ───────────────────────────────────
    int cur_len;
    int max_len;
    int growth_cnt;
    int poison_cnt;
    int gate_cnt;

    // ── 미션 달성 플래그 ────────────────────────
    bool len_ok, growth_ok, poison_ok, gate_ok;

    // ── 타이머 ─────────────────────────────────
    std::chrono::steady_clock::time_point start_time;      // 스테이지 시작 시각
    std::chrono::steady_clock::time_point item_spawn_time; // 아이템 마지막 재배치 시각
    std::chrono::steady_clock::time_point gate_spawn_time; // 게이트 마지막 재생성 시각

    // 특수 아이템 활성화 시각 (-1초: 비활성)
    std::chrono::steady_clock::time_point immune_start;
    std::chrono::steady_clock::time_point slow_start;
    bool immune_active; // 무적 아이템 활성 여부
    bool slow_active;   // 속도 느려지는 아이템 활성 여부

    // ── 객체 ───────────────────────────────────
    Snake    snake;
    Map      map;
    Contents contents;
    Gate     gate;

    // ── 내부 함수 ──────────────────────────────
    void init_screen();          // ncurses 화면 초기화
    void init_stage();           // 스테이지 초기화 (맵·뱀·아이템·게이트 리셋)
    void update_screen();        // 화면 갱신
    void process_input();        // 키 입력 처리
    void update_state();         // 게임 상태 갱신 (이동·충돌·아이템)
    void update_timers();        // 타이머 기반 이벤트 처리
    void update_contents_val();  // 스코어보드 값 갱신

    void spawn_items();          // Growth·Poison·특수 아이템 초기 배치
    void respawn_items();        // 아이템 5초마다 재배치
    void place_item(int cell_value); // 맵의 빈 칸에 지정 셀 값 배치

    void remove_items_from_map();    // 맵에서 모든 아이템 제거
    void remove_gates_from_map();    // 맵에서 게이트 제거 후 벽 복원

    bool check_mission() const;  // 미션 전체 달성 여부
    void next_stage();           // 다음 스테이지로 전환
    void show_stage_clear();     // 스테이지 클리어 화면 출력
    void game_over();            // 게임 오버 화면 출력
    void show_all_clear();       // 전체 클리어 화면 출력

    MissionGoal get_mission_goal() const; // 현재 스테이지 미션 목표 반환
};
