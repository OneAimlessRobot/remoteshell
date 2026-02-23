#ifndef ADMIN_PTY_SETTING_H
#define ADMIN_PTY_SETTING_H

#define DEFAULT_TERM_HEIGHT 128
#define DEFAULT_TERM_WIDTH 128


extern uint32_t height_for_pty,
                width_for_pty;

extern struct winsize the_winsize_struct;

void set_pty_size(int pty_master_fd);

#endif
