#include "gridlinelayouter.h"

void GridLineLayouter::calculateSize(int maxW, int maxH, size_t count)
{
    resize(maxW * count, maxH);
}
