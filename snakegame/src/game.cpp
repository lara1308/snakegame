// ────────────────────────────────────────────────
// game.cpp
// 게임 루프, 스테이지 전환, 아이템 배치·획득 처리,
// 타이머 관리, 충돌 처리를 담당하는 Game 클래스 구현
// ────────────────────────────────────────────────

#include "game.hpp"
#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <cstdio>

// ── 색상 인덱스 상수 ─────────────────────────────────────────────────────────
static const int COLOR_ORANGE = 8;  // 커스텀 색상: 주황
static const int COLOR_GRAY   = 9;  // 커스텀 색상: 회색
static const int COLOR_CYAN_BG  = 10; // 커스텀 색상: 하늘색 배경 (무적 아이템)
// 노란색(COLOR_YELLOW)은 ncurses 기본 제공

// 컬러 페어 인덱스
static const int PAIR_EMPTY   = 1; // 빈 칸 (흰 배경)
static const int PAIR_WALL    = 2; // 일반 벽 (회색)
static const int PAIR_IWALL   = 3; // 면역 벽 (검정)
static const int PAIR_HEAD    = 4; // 뱀 머리 (노란 배경)
static const int PAIR_BODY    = 5; // 뱀 몸통 (주황 배경)
static const int PAIR_GROWTH  = 6; // Growth Item (초록)
static const int PAIR_POISON  = 7; // Poison Item (빨강)
static const int PAIR_GATE    = 8; // Gate (마젠타)
static const int PAIR_SCORE   = 9; // 스코어보드 글자 (흰 배경, 검정 글자)
static const int PAIR_IMMUNE  = 10; // 무적 아이템 (하늘색)
static const int PAIR_SLOW    = 11; // 속도 아이템 (노란색)

// 스테이지별 미션 목표값
static const MissionGoal MISSION_GOALS[4] = {
    { 5,  3, 2, 1}, // 스테이지 1
    { 7,  5, 3, 2}, // 스테이지 2
    {10,  7, 4, 3}, // 스테이지 3
    {13, 10, 5, 4}, // 스테이지 4
};

// 내부 벽 붕괴 주기 (스테이지 3·4 한정, 단위: 초)
static const double WALL_COLLAPSE_INTERVAL = 15.0;

// 아이템 위치 갱신 주기 (초)
static const double ITEM_RESPAWN_INTERVAL = 15.0;

// 게이트 재생성 주기 (초)
static const double GATE_RESPAWN_INTERVAL = 10.0;

// 특수 아이템 지속 시간 (초)
static const double IMMUNE_ITEM_DURATION = 8.0;  // 무적 아이템 지속 시간 (초)
static const double SLOW_ITEM_DURATION = 8.0;   // 속도 아이템 지속 시간 (초)

// 방향 오프셋 (UP=0, DOWN=1, LEFT=2, RIGHT=3)
static const int MOVE_DX[4] = {-1, 1,  0,  0};
static const int MOVE_DY[4] = { 0, 0, -1,  1};

// ── 생성자 ───────────────────────────────────────────────────────────────────

Game::Game(int map_index, int tick_ms)
    : current_stage(map_index),
      tick_ms(tick_ms),
      is_running(false),
      quit_requested(false),
      cur_len(0), max_len(0),
      growth_cnt(0), poison_cnt(0), gate_cnt(0),
      len_ok(false), growth_ok(false), poison_ok(false), gate_ok(false),
      immune_active(false), slow_active(false),
      next_collapse(WALL_COLLAPSE_INTERVAL),
      snake(),
      map(30, 30, map_index),
      contents(),
      gate()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

// ── ncurses 화면 초기화 ──────────────────────────────────────────────────────

void Game::init_screen() {
    initscr();
    start_color();
    noecho();
    cbreak();

    if (!has_colors()) {
        endwin();
        return;
    }

    // 커스텀 색상 정의 (터미널이 지원하는 경우)
    if (can_change_color()) {
        init_color(COLOR_GRAY,  500, 500, 500);       // 회색
        init_color(COLOR_ORANGE, 1000, 500, 0);       // 주황
        init_color(COLOR_BLACK, 0, 0, 0);             // 검정
        init_color(COLOR_CYAN_BG, 0, 800, 800);       // 하늘색
    }

    // 컬러 페어 정의
    init_pair(PAIR_EMPTY,  COLOR_WHITE,  COLOR_WHITE);  // 빈 칸
    init_pair(PAIR_WALL,   COLOR_BLACK,  COLOR_GRAY);   // 일반 벽
    init_pair(PAIR_IWALL,  COLOR_BLACK,  COLOR_BLACK);  // 면역 벽
    init_pair(PAIR_HEAD,   COLOR_BLACK,  COLOR_YELLOW); // 뱀 머리
    init_pair(PAIR_BODY,   COLOR_BLACK,  COLOR_ORANGE); // 뱀 몸통
    init_pair(PAIR_GROWTH, COLOR_BLACK,  COLOR_GREEN);  // Growth Item
    init_pair(PAIR_POISON, COLOR_BLACK,  COLOR_RED);    // Poison Item
    init_pair(PAIR_GATE,   COLOR_BLACK,  COLOR_MAGENTA);// Gate
    init_pair(PAIR_SCORE,  COLOR_BLACK,  COLOR_WHITE);  // 스코어보드
    init_pair(PAIR_IMMUNE, COLOR_BLACK,  COLOR_CYAN_BG);// 무적 아이템 (하늘색)
    init_pair(PAIR_SLOW,   COLOR_BLACK,  COLOR_YELLOW); // 속도 아이템 (노란색)

    bkgd(COLOR_PAIR(PAIR_EMPTY));
}

// ── 스테이지 초기화 ──────────────────────────────────────────────────────────

void Game::init_stage() {
    // 맵 재생성
    map = Map(30, 30, current_stage);

    // 뱀을 맵 데이터의 3/4 좌표로 초기화
    snake = Snake(map);

    // 카운터 초기화
    growth_cnt = 0;
    poison_cnt = 0;
    gate_cnt   = 0;
    max_len    = static_cast<int>(snake.get_body().size());
    cur_len    = max_len;
    len_ok = growth_ok = poison_ok = gate_ok = false;

    // 특수 아이템 비활성화
    immune_active = false;
    slow_active   = false;

    // 타이머 초기화
    const auto now = std::chrono::steady_clock::now();
    next_collapse = WALL_COLLAPSE_INTERVAL;
    start_time      = now;
    item_spawn_time = now;
    gate_spawn_time = now;

    // 아이템 초기 배치
    spawn_items();

    // 게이트 최초 생성
    gate.spawn(map.getMapData());

    // 스코어보드 스테이지 번호 갱신 (1~4 표시)
    contents.set_stage(current_stage + 1);
}

// ── 미션 목표 반환 ───────────────────────────────────────────────────────────

MissionGoal Game::get_mission_goal() const {
    return MISSION_GOALS[current_stage];
}

// ── 아이템 배치 ──────────────────────────────────────────────────────────────

// 빈 칸(0)에 지정 셀 값을 무작위로 배치
void Game::place_item(int cell_value) {
    const int rows = map.getHeight();
    const int cols = map.getWidth();
    int attempts = 0;
    while (attempts < 1000) {
        const int x = std::rand() % (rows - 2) + 1; // 겉 벽 제외
        const int y = std::rand() % (cols - 2) + 1;
        if (map.getMapData()[x][y] == 0) {
            map.getMapData()[x][y] = cell_value;
            return;
        }
        ++attempts;
    }
}

// 맵에서 아이템(5,6,8,9) 모두 제거
void Game::remove_items_from_map() {
    for (auto& row : map.getMapData()) {
        for (auto& cell : row) {
            if (cell == 5 || cell == 6 || cell == 8 || cell == 9) {
                cell = 0;
            }
        }
    }
}

// 맵에서 게이트(7)를 일반 벽(1)으로 복원
void Game::remove_gates_from_map() {
    for (auto& row : map.getMapData()) {
        for (auto& cell : row) {
            if (cell == 7) {
                cell = 1;
            }
        }
    }
}

// Growth Item(5) + Poison Item(6) + 스테이지별 특수 아이템 초기 배치
void Game::spawn_items() {
    place_item(5); // Growth Item
    place_item(6); // Poison Item

    // 스테이지 1·2: 속도 느려지는 아이템(9, 노란색)
    if (current_stage == 0 || current_stage == 1) {
        place_item(9);
    }
    // 스테이지 3·4: 무적 아이템(8, 하늘색)
    if (current_stage == 2 || current_stage == 3) {
        place_item(8);
    }
}

// 5초마다 아이템 위치 재배치
void Game::respawn_items() {
    remove_items_from_map();
    spawn_items();
    item_spawn_time = std::chrono::steady_clock::now();
}

// ── 미션 달성 확인 ───────────────────────────────────────────────────────────

bool Game::check_mission() const {
    return len_ok && growth_ok && poison_ok && gate_ok;
}

// ── 스코어보드 값 갱신 ───────────────────────────────────────────────────────

void Game::update_contents_val() {
    cur_len = snake.get_length();
    if (cur_len > max_len) max_len = cur_len;

    const MissionGoal goal = get_mission_goal();
    len_ok    = (cur_len    >= goal.body);
    growth_ok = (growth_cnt >= goal.growth);
    poison_ok = (poison_cnt >= goal.poison);
    gate_ok   = (gate_cnt   >= goal.gate);

    // 특수 아이템 남은 시간 계산
    const auto now = std::chrono::steady_clock::now();
    double immune_remain = -1.0;
    double slow_remain   = -1.0;

    if (immune_active && (current_stage == 2 || current_stage == 3)) {
        const double elapsed =
            std::chrono::duration<double>(now - immune_start).count();
        immune_remain = IMMUNE_ITEM_DURATION - elapsed;
        if (immune_remain < 0.0) immune_remain = 0.0;
    }
    if (slow_active && (current_stage == 0 || current_stage == 1)) {
        const double elapsed =
            std::chrono::duration<double>(now - slow_start).count();
        slow_remain = SLOW_ITEM_DURATION - elapsed;
        if (slow_remain < 0.0) slow_remain = 0.0;
    }

    contents.set_status(cur_len, max_len, growth_cnt, poison_cnt, gate_cnt);
    contents.set_mission(goal.body, goal.growth, goal.poison, goal.gate,
                         len_ok, growth_ok, poison_ok, gate_ok);
    // 게이트 재생성까지 남은 시간 계산
    const double gate_elapsed_remain =
        std::chrono::duration<double>(now - gate_spawn_time).count();
    const double gate_remain = GATE_RESPAWN_INTERVAL - gate_elapsed_remain;

    contents.set_item_timer(immune_remain, slow_remain, gate_remain > 0.0 ? gate_remain : 0.0);
}

// ── 화면 갱신 ────────────────────────────────────────────────────────────────

void Game::update_screen() {
    clear();

    const auto& mapData = map.getMapData();
    const int rows = map.getHeight();
    const int cols = map.getWidth();

    // 맵 출력 (각 셀을 2칸 너비로 출력)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const int cell = mapData[i][j];
            int color = PAIR_EMPTY;

            switch (cell) {
                case 0: color = PAIR_EMPTY;  break; // 빈 칸
                case 1: color = PAIR_WALL;   break; // 일반 벽
                case 2: color = PAIR_IWALL;  break; // 면역 벽
                case 3: color = PAIR_HEAD;   break; // 뱀 머리
                case 4: color = PAIR_BODY;   break; // 뱀 몸통
                case 5: color = PAIR_GROWTH; break; // Growth Item
                case 6: color = PAIR_POISON; break; // Poison Item
                case 7: color = PAIR_GATE;   break; // Gate
                case 8: color = PAIR_IMMUNE; break; // 무적 아이템
                case 9: color = PAIR_SLOW;   break; // 속도 아이템
                default: color = PAIR_EMPTY; break;
            }

            attron(COLOR_PAIR(color));
            mvaddch(i, j * 2,     ' ');
            mvaddch(i, j * 2 + 1, ' ');
            attroff(COLOR_PAIR(color));
        }
    }

    // 스코어보드 갱신 후 출력
    update_contents_val();
    int term_rows, term_cols;
    getmaxyx(stdscr, term_rows, term_cols);
    const int score_x = cols * 2 + 2;
    if (score_x + 20 <= term_cols) {
        contents.draw(score_x, 1);
    } else {
        contents.draw(0, rows + 1);
    }
    (void)term_rows;

    refresh();
}

// ── 키 입력 처리 ─────────────────────────────────────────────────────────────

void Game::process_input() {
    const int ch = getch();
    switch (ch) {
        case 'w': case 'W': case KEY_UP:    snake.set_direction(UP);    break;
        case 's': case 'S': case KEY_DOWN:  snake.set_direction(DOWN);  break;
        case 'a': case 'A': case KEY_LEFT:  snake.set_direction(LEFT);  break;
        case 'd': case 'D': case KEY_RIGHT: snake.set_direction(RIGHT); break;
        case 'q': case 'Q':
            is_running    = false;
            quit_requested = true;
            break;
        default: break;
    }
}

// ── 타이머 기반 이벤트 ───────────────────────────────────────────────────────

void Game::update_timers() {
    const auto now = std::chrono::steady_clock::now();

    // 아이템 5초마다 재배치
    const double item_elapsed =
        std::chrono::duration<double>(now - item_spawn_time).count();
    if (item_elapsed >= ITEM_RESPAWN_INTERVAL) {
        respawn_items();
    }

    // 게이트 10초마다 재생성
    const double gate_elapsed =
        std::chrono::duration<double>(now - gate_spawn_time).count();
    if (gate_elapsed >= GATE_RESPAWN_INTERVAL) {
        gate.respawn(map.getMapData());
        gate_spawn_time = now;
    }

    // 스테이지 3·4: 내부 벽 붕괴 (WALL_COLLAPSE_INTERVAL 주기)
    if (current_stage == 2 || current_stage == 3) {
        const double stage_elapsed =
            std::chrono::duration<double>(now - start_time).count();
        // 15초, 30초, ... 경과할 때마다 남은 내부 벽 한 번 더 제거
        if (stage_elapsed >= next_collapse) {
            map.collapseInnerWalls();
            next_collapse += WALL_COLLAPSE_INTERVAL;
        }
    }

    // 무적 아이템 3초 만료
    if (immune_active) {
        const double elapsed =
            std::chrono::duration<double>(now - immune_start).count();
        if (elapsed >= IMMUNE_ITEM_DURATION) {
            immune_active = false;
        }
    }

    // 속도 아이템 3초 만료
    if (slow_active) {
        const double elapsed =
            std::chrono::duration<double>(now - slow_start).count();
        if (elapsed >= SLOW_ITEM_DURATION) {
            slow_active = false;
            tick_ms = 150; // 속도 원복
        }
    }
}

// ── 게임 상태 갱신 ───────────────────────────────────────────────────────────

void Game::update_state() {
    // 방향에 따른 오프셋
    const int dx = MOVE_DX[snake.get_direction()];
    const int dy = MOVE_DY[snake.get_direction()];

    // 이전 뱀 위치를 맵에서 제거
    for (const auto& part : snake.get_body()) {
        const int x = part.first;
        const int y = part.second;
        if (x >= 0 && x < map.getHeight() && y >= 0 && y < map.getWidth()) {
            const int cell = map.getMapData()[x][y];
            if (cell == 3 || cell == 4) {
                map.getMapData()[x][y] = 0;
            }
        }
    }

    // 뱀 이동
    snake.move(dx, dy);

    // 뱀이 반대 방향 입력으로 사망했으면 즉시 종료
    if (!snake.get_alive()) {
        is_running = false;
        return;
    }

    auto head = snake.get_head();

    // ── 게이트 진입 확인
    if (gate.is_gate(head.first, head.second)) {
        const auto result = gate.teleport_with_dir(
            head.first, head.second, snake.get_direction(), map.getMapData());
        snake.set_head(result.first.first, result.first.second);
        snake.set_direction(result.second);
        head = snake.get_head();
        gate_cnt++;
    }

    // ── 벽/면역벽 충돌
    if (head.first < 0 || head.first >= map.getHeight() ||
        head.second < 0 || head.second >= map.getWidth())
    {
        //맵 바깥 : 무적 여부 상관없이 겉 벽이므로 사망
        snake.set_alive(false);
        is_running = false;
        return;  
    } else {
        const int cell_at_head = map.getMapData()[head.first][head.second];
        if (cell_at_head == 1 || cell_at_head == 2) {
            if (immune_active && cell_at_head == 1) {
                // 벽에 부딪혔지만 사망하지 않음; 머리를 이전 위치로 되돌림
                snake.set_head(head.first - dx, head.second - dy);
                head = snake.get_head();
            } else {
                snake.set_alive(false);
                is_running = false;
                return;
            }
        }
    }

    // ── 자기 몸 충돌
    if (snake.check_collision()) {
        snake.set_alive(false);
        is_running = false;
        return;
    }

    // ── 아이템 획득 처리
    const int item = map.getMapData()[head.first][head.second];

    if (item == 5) { // Growth Item
        growth_cnt++;
        snake.grow();
        map.getMapData()[head.first][head.second] = 0;
        place_item(5); // 다른 위치에 새 Growth Item 생성

    } else if (item == 6) { // Poison Item
        poison_cnt++;
        if (!immune_active) {
            if (snake.get_length() > 3) {
                snake.shrink();
            } else {
                snake.set_alive(false);
                is_running = false;
                return;
            }
        }
        map.getMapData()[head.first][head.second] = 0;
        place_item(6); // 다른 위치에 새 Poison Item 생성

    } else if (item == 8) { // 무적 아이템 (스테이지 1·2)
        immune_active = true;
        immune_start  = std::chrono::steady_clock::now();
        map.getMapData()[head.first][head.second] = 0;
        place_item(8); // 새 위치에 재배치

    } else if (item == 9) { // 속도 느려지는 아이템 (스테이지 3·4)
        slow_active = true;
        slow_start  = std::chrono::steady_clock::now();
        tick_ms = 300; // 속도 절반으로 감소
        map.getMapData()[head.first][head.second] = 0;
        place_item(9); // 새 위치에 재배치
    }

    // ── 뱀의 새 위치를 맵에 표시
    const auto& body = snake.get_body();
    for (size_t i = 0; i < body.size(); ++i) {
        const int x = body[i].first;
        const int y = body[i].second;
        if (x >= 0 && x < map.getHeight() && y >= 0 && y < map.getWidth()) {
            map.getMapData()[x][y] = (i == 0) ? 3 : 4;
        }
    }
}

// ── 스테이지 클리어 화면 ─────────────────────────────────────────────────────

void Game::show_stage_clear() {
    clear();
    const int cy = map.getHeight() / 2;
    const int cx = map.getWidth();
    mvprintw(cy,     cx, "  Stage %d Clear!  ", current_stage + 1);
    mvprintw(cy + 1, cx, "Next stage starts...");
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// ── 다음 스테이지 전환 ───────────────────────────────────────────────────────

void Game::next_stage() {
    show_stage_clear();
    current_stage++;
    // 내부 벽 붕괴 타이머 리셋 (static 변수 초기화)
    // game.cpp 내 update_timers()의 next_collapse를 리셋
    // → init_stage()에서 start_time을 현재 시각으로 갱신하므로
    //   next_collapse도 함께 리셋
    init_stage();
}

// ── 전체 클리어 화면 ─────────────────────────────────────────────────────────

void Game::show_all_clear() {
    clear();
    const int cy = map.getHeight() / 2;
    const int cx = map.getWidth() - 5;
    mvprintw(cy,     cx, "===  ALL CLEAR!  ===");
    mvprintw(cy + 1, cx, "Congratulations!");
    mvprintw(cy + 3, cx, "Press any key...");
    refresh();
    getch();
}

// ── 게임 오버 화면 ───────────────────────────────────────────────────────────

void Game::game_over() {
    clear();
    const int cy = map.getHeight() / 2;
    const int cx = map.getWidth() - 5;

    mvprintw(cy,     cx, "  GAME OVER  ");
    mvprintw(cy + 1, cx, "Press any key...");

    // 최종 통계 출력
    const int iy = cy + 3;
    mvprintw(iy,     cx, "Stage  : %d", current_stage + 1);
    mvprintw(iy + 1, cx, "B max  : %d", max_len);
    mvprintw(iy + 2, cx, "Growth : %d", growth_cnt);
    mvprintw(iy + 3, cx, "Poison : %d", poison_cnt);
    mvprintw(iy + 4, cx, "Gate   : %d", gate_cnt);

    // 경과 시간
    const auto end_time = std::chrono::steady_clock::now();
    const long long sec =
        std::chrono::duration_cast<std::chrono::seconds>(
            end_time - start_time).count();
    mvprintw(iy + 5, cx, "Time   : %lldsec", sec);

    refresh();
    getch();
    endwin();
}

// ── 메인 게임 루프 ───────────────────────────────────────────────────────────

void Game::run() {
    init_screen();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    init_stage(); // 첫 스테이지 초기화

    is_running = true;

    while (true) {
        // ── 스테이지 루프
        while (is_running) {
            process_input();

            if (!is_running) break; // q 키 처리

            update_timers();

            update_state();
            if(!is_running) break; // 충돌로 종료

            update_contents_val();
            if (check_mission()) {
                if (current_stage < 3) {
                    next_stage();
                } else {
                    show_all_clear();
                    endwin();
                    return;
                }
            }

            update_screen();
            napms(tick_ms);
        }

        // q 키로 강제 종료
        if (quit_requested) break;

        // 뱀이 살아있고 미션 달성 → 다음 스테이지
        if (check_mission() && current_stage < 3) {
            next_stage();
            is_running = true; // 다음 스테이지 루프 재개
            continue;
        }

        // 4스테이지 미션 모두 달성 → 전체 클리어
        if (check_mission() && current_stage == 3) {
            show_all_clear();
            break;
        }

        // 뱀이 죽거나 기타 종료 조건
        break;
    }

    game_over();
}