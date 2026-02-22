#ifndef ADMIN_CERT_FILE_PATHS_H
#define ADMIN_CERT_FILE_PATHS_H

extern const char
       * admin_auth_cert_file_path,
       * admin_host_cert_file_path,
       * admin_host_pkey_file_path;


void initalize_admin_cert_file_paths(void);

#endif
