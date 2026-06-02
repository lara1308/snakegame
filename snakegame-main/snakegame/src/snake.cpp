// ────────────────────────────────────────────────
// snake.cpp
// 뱀의 초기화, 이동, 방향 전환, 성장·수축,
// 벽·자기 몸 충돌 검사를 담당하는 Snake 클래스 구현
// ────────────────────────────────────────────────

#include "snake.hpp"
#include <vector>
#include <utility>

// 방향 상수 (game.hpp의 Direction enum과 동일)
enum SnakeDir { S_UP = 0, S_DOWN = 1, S_LEFT = 2, S_RIGHT = 3 };

// ── 생성자 ───────────────────────────────────────────────────────────────────

// (x, y) 좌표에서 길이 3으로 초기화 (오른쪽 방향)
Snake::Snake(int x, int y) {
    body.clear();
    body.push_back({x, y});
    body.push_back({x, y - 1});
    body.push_back({x, y - 2});
    direction = S_RIGHT;
    length    = 3;
    score     = 0;
    alive     = true;
}

// 맵 데이터에서 3(머리), 4(몸통) 좌표를 읽어 뱀을 초기화
Snake::Snake(const Map& map) {
    body.clear();
    const int rows = map.getHeight();
    const int cols = map.getWidth();

    // 머리(3) 먼저 찾기
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            if (map.getMapData()[x][y] == 3) {
                body.insert(body.begin(), {x, y});
            }
        }
    }
    // 몸통(4)은 순서대로 뒤에 추가
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            if (map.getMapData()[x][y] == 4) {
                body.push_back({x, y});
            }
        }
    }

    // 초기 방향: 머리에서 두 번째 몸통을 보고 결정
    direction = S_RIGHT; // 기본값
    if (body.size() >= 2) {
        int dx = body[0].first  - body[1].first;
        int dy = body[0].second - body[1].second;
        if      (dx == -1) direction = S_UP;
        else if (dx ==  1) direction = S_DOWN;
        else if (dy == -1) direction = S_LEFT;
        else if (dy ==  1) direction = S_RIGHT;
    }

    length = static_cast<int>(body.size());
    score  = 0;
    alive  = true;
}

// ── 방향 설정 ────────────────────────────────────────────────────────────────

void Snake::set_direction(int dir) {
    // 반대 방향 입력 시 즉사 (과제 규칙 #1)
    const bool opposite =
        (direction == S_UP    && dir == S_DOWN)  ||
        (direction == S_DOWN  && dir == S_UP)    ||
        (direction == S_LEFT  && dir == S_RIGHT) ||
        (direction == S_RIGHT && dir == S_LEFT);

    if (opposite) {
        alive = false; // 반대 방향 입력 → 실패
        return;
    }
    direction = dir;
}

// ── 이동 ─────────────────────────────────────────────────────────────────────

void Snake::move(int dx, int dy) {
    if (!alive) return;

    // 새 머리 좌표 계산 후 맨 앞에 삽입
    const int new_x = body.front().first  + dx;
    const int new_y = body.front().second + dy;
    body.insert(body.begin(), {new_x, new_y});

    // 목표 길이보다 길면 꼬리 제거
    if (static_cast<int>(body.size()) > length) {
        body.pop_back();
    }
}

// ── 성장/수축 ────────────────────────────────────────────────────────────────

void Snake::grow() {
    length++; // 다음 이동 시 꼬리를 제거하지 않아 자동으로 길어짐
    score += 10;
}

void Snake::shrink() {
    if (static_cast<int>(body.size()) > 1) {
        body.pop_back();
        length--;
        // 길이가 3 미만이면 게임 오버 (과제 규칙 #2)
        if (length < 3) {
            alive = false;
        }
    }
}

// ── 충돌 검사 ────────────────────────────────────────────────────────────────

bool Snake::check_collision() const {
    // 머리가 자신의 몸통과 겹치는지 검사
    const auto& head = body.front();
    for (size_t i = 1; i < body.size(); ++i) {
        if (body[i] == head) {
            return true;
        }
    }
    return false;
}

// ── 머리 좌표 직접 설정 (게이트 텔레포트 시 사용) ──────────────────────────

void Snake::set_head(int x, int y) {
    if (!body.empty()) {
        body[0] = {x, y};
    }
}
