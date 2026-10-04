#ifndef CLIENT_PTY_SETTING_H
#define CLIENT_PTY_SETTING_H

#define DEFAULT_TERM_HEIGHT 80
#define DEFAULT_TERM_WIDTH 80

extern uint32_t height_for_pty,
                 width_for_pty;

extern struct winsize the_winsize_struct;

void set_pty_size(int pty_master_fd);

#endif
