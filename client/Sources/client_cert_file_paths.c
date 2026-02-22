#include "../../xtrafun/Includes/preprocessor.h"
#include "../../xtrafun/Includes/fileshit.h"
#include "../Includes/client_cert_file_paths.h"


const char* client_auth_cert_file_path = "./certs_and_pkeys/remoteshell_auth.crt",
	* client_host_cert_file_path = "./certs_and_pkeys/remoteshell_auth_host_client.crt",
	* client_host_pkey_file_path = "./certs_and_pkeys/remoteshell_auth_host_client.key";

void initalize_client_cert_file_paths(void){

        memcpy(auth_cert_file_path,client_auth_cert_file_path,min(strlen(client_auth_cert_file_path),sizeof(auth_cert_file_path)));

        auth_cert_file_path[sizeof(auth_cert_file_path)-1]=0;



        memcpy(host_cert_file_path,client_host_cert_file_path,min(strlen(client_host_cert_file_path),sizeof(host_cert_file_path)));

        host_cert_file_path[sizeof(host_cert_file_path)-1]=0;



        memcpy(host_pkey_file_path,client_host_pkey_file_path,min(strlen(client_host_pkey_file_path),sizeof(host_pkey_file_path)));

        host_pkey_file_path[sizeof(host_pkey_file_path)-1]=0;

}
