#pragma once
#include <vector>

// 맵 셀 값 상수 정의
// 0: 빈 칸, 1: 일반 벽(Wall), 2: 면역 벽(Immune Wall)
// 3: 뱀 머리, 4: 뱀 몸통
// 5: Growth Item, 6: Poison Item, 7: Gate
// 8: 무적 아이템(Immune Item), 9: 속도 느려지는 아이템(Slow Item)

class Map {
public:
    Map(int width = 30, int height = 30, int map_index = 0);

    const std::vector<std::vector<int>>& getMapData() const;
    std::vector<std::vector<int>>& getMapData();

    void generate(); // 맵 생성 함수

    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    // 3번 맵(index=2)의 내부 벽 붕괴 처리
    void collapseInnerWalls();

    // 해당 좌표가 일반 벽(Gate로 변할 수 있는 벽)인지 반환
    bool isNormalWall(int x, int y) const;

private:
    int width, height;
    int map_index;
    std::vector<std::vector<int>> mapData;
};
