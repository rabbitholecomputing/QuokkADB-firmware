#ifdef ADBUINO
#include "adbuino.cpp"
#endif

#ifdef QUOKKADB
#include "quokkadb.h"
int main(void) {
    return quokkadb();
}
#elif QUOKKAMIGA
#include "quokkamiga.h"
int main(void) {
    return quokkamiga();
}
#endif
