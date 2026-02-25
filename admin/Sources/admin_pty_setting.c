#include "../../xtrafun/Includes/preprocessor.h"
#include "../../xtrafun/Includes/fileshit.h"
#include "../Includes/admin_pty_setting.h"

uint32_t height_for_pty=DEFAULT_TERM_HEIGHT,
		width_for_pty=DEFAULT_TERM_WIDTH;

struct winsize the_winsize_struct={DEFAULT_TERM_HEIGHT,DEFAULT_TERM_WIDTH,0,0};

void set_pty_size(int pty_master_fd) {

    the_winsize_struct.ws_row=height_for_pty;
    the_winsize_struct.ws_col=width_for_pty;

    // Use ioctl to set the size on the master PTY
    if (ioctl(pty_master_fd, TIOCSWINSZ, &the_winsize_struct) == -1) {
        perror("ioctl TIOCSWINSZ");
        exit(1);
    }
}
