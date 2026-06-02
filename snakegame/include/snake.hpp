// ────────────────────────────────────────────────
// snake.hpp
// 뱀의 이동, 방향 전환, 성장·수축,
// 충돌 검사를 담당하는 Snake 클래스 선언
// ────────────────────────────────────────────────

#pragma once

#include <vector>
#include <utility>
#include "map.hpp"

class Snake {
public:
    Snake() : direction(3), length(3), score(0), alive(true) {} // 기본 생성자 (방향: RIGHT)
    Snake(int x, int y);
    Snake(const Map& map);

    void move(int dx, int dy);
    void grow();
    void shrink();
    bool check_collision() const;

    void set_direction(int dir);
    int  get_direction() const { return direction; }

    std::pair<int,int> get_head() const { return body.front(); }
    void set_head(int x, int y);

    void set_alive(bool val) { alive = val; }
    bool get_alive() const   { return alive; }

    const std::vector<std::pair<int,int>>& get_body() const { return body; }
    int get_length() const { return static_cast<int>(body.size()); }

private:
    std::vector<std::pair<int,int>> body; // 뱀의 몸통 좌표 (앞=머리)
    int direction; // 현재 방향 (UP=0, DOWN=1, LEFT=2, RIGHT=3)
    int length;    // 목표 길이 (grow/shrink로 조절)
    int score;     // 점수
    bool alive;    // 생존 여부
};
