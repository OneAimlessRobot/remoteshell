#include "../../xtrafun/Includes/preprocessor.h"
#include "../../xtrafun/Includes/fileshit.h"
#include "../Includes/admin_cert_file_paths.h"

const char* admin_auth_cert_file_path = "./certs_and_pkeys/remoteshell_auth.crt",
        * admin_host_cert_file_path = "./certs_and_pkeys/remoteshell_auth_host_admin.crt",
        * admin_host_pkey_file_path = "./certs_and_pkeys/remoteshell_auth_host_admin.key";


void initalize_admin_cert_file_paths(void){

	memcpy(auth_cert_file_path,admin_auth_cert_file_path,min(strlen(admin_auth_cert_file_path),sizeof(auth_cert_file_path)));

	auth_cert_file_path[sizeof(auth_cert_file_path)-1]=0;



	memcpy(host_cert_file_path,admin_host_cert_file_path,min(strlen(admin_host_cert_file_path),sizeof(host_cert_file_path)));

	host_cert_file_path[sizeof(host_cert_file_path)-1]=0;



	memcpy(host_pkey_file_path,admin_host_pkey_file_path,min(strlen(admin_host_pkey_file_path),sizeof(host_pkey_file_path)));

	host_pkey_file_path[sizeof(host_pkey_file_path)-1]=0;

}
