#include <stdio.h>
#include <math.h>

typedef struct P {
    int x, y;
} Point;

double getDist(const Point a, const Point b) {
    int xDiff = b.x - a.x;
    int yDiff = b.y - a.y;
    return sqrt(xDiff * xDiff + yDiff * yDiff);
}

int main() {
    int n;
    scanf("%d", &n);

    Point firstPoint;
    scanf("%d %d", &firstPoint.x, &firstPoint.y);

    Point prevPoint = firstPoint;
    Point currPoint;
    double dists = 0.0;

    for (int i = 1; i < n; ++i) {
        scanf("%d %d", &currPoint.x, &currPoint.y);
        dists += getDist(prevPoint, currPoint);
        prevPoint = currPoint;
    }
    dists += getDist(currPoint, firstPoint);

    printf("%d\n", (int) ceil(dists * 5.0));
    return 0;
}
