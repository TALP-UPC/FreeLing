
#if defined WIN32 || defined WIN64
#include    <io.h>     
#include    <fcntl.h>
static void revert_mode()
{
    _setmode( _fileno(stdin),  _O_TEXT );
    _setmode( _fileno(stdout), _O_TEXT );
}
#endif
