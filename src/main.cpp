#include "Miru.h"

int main() {

    const Miru miru;

    while (miru.shouldRun())
        miru.update();

    return 0;
}
