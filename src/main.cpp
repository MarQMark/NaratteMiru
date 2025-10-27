#include "Miru.h"

int main() {

    Miru miru;

    while (miru.shouldRun())
        miru.update();

    return 0;
}
