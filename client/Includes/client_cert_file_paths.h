#ifndef CLIENT_CERT_FILE_PATHS_H
#define CLIENT_CERT_FILE_PATHS_H

extern const char
	* client_auth_cert_file_path,
	* client_host_cert_file_path,
	* client_host_pkey_file_path;


void initalize_client_cert_file_paths(void);
#endif
