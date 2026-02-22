#!/bin/bash

name_string_for_authority_stuff="remoteshell_auth"

host_string_in_file_name="host"
tmp_auth_cert_directory_name="tmp_auth_cert_dir"

tmp_host_cert_directory_name="tmp_host_cert_dir"

our_cwd="$(pwd)/"

script_root_name="/cert_script_root/"

module_cert_directory_name="/certs_and_pkeys/"

directories_to_work_on=("admin" "client")

complete_directories_to_work_on=()

complete_cert_directories_to_work_on=()

echo "$our_cwd"

total=${#directories_to_work_on[@]}

echo ${total}

print_directories(){
	for((i=0; i<$total; i++)); 
	do
		echo "$our_cmd${directories_to_work_on[$i]}"
	done
}

fill_complete_directories(){
	for ((i=0; i<total; i++));
	do
		complete_directories_to_work_on[i]="${our_cwd}${directories_to_work_on[$i]}"
	done
}

fill_complete_cert_directories(){
	for ((i=0; i<total; i++));
	do
		complete_cert_directories_to_work_on[i]="${complete_directories_to_work_on[$i]}$module_cert_directory_name"
	done
}

print_complete_directories(){
	for((i=0; i<$total; i++)); 
	do
		echo "${complete_directories_to_work_on[$i]}"
	done
}

print_complete_cert_directories(){
	for((i=0; i<$total; i++)); 
	do
		echo "${complete_cert_directories_to_work_on[$i]}"
	done
}


cre8_cert_directories(){
	echo "cre8_cert_directories()"
	
	for((i=0; i<$total; i++)); 
	do
		mkdir "${complete_cert_directories_to_work_on[$i]}"
	done
}

cre8_cert_module_name_files(){
	for((i=0; i<$total; i++)); 
	do
		cp "${name_string_for_authority_stuff}_${host_string_in_file_name}.conf" "${complete_cert_directories_to_work_on[$i]}${name_string_for_authority_stuff}_${host_string_in_file_name}_${directories_to_work_on[$i]}.conf"
	done
}


remv_cert_directories(){
	for((i=0; i<$total; i++)); 
	do
		rm -rvf "${complete_cert_directories_to_work_on[$i]}"
	done
	
}
fill_complete_directories

print_complete_directories

fill_complete_cert_directories

print_complete_cert_directories


	
gener8_auth_cert(){

	openssl genrsa -out "${name_string_for_authority_stuff}.key" 4096
	 
	openssl req -x509 -new -nodes -key "${name_string_for_authority_stuff}.key" -out "${name_string_for_authority_stuff}.crt" -days 3650 -config "${name_string_for_authority_stuff}.conf"

	echo "Verifying CA certificate..."
	
	openssl x509 -in "${name_string_for_authority_stuff}.crt" -text -noout
}


cre8_auth_cert(){
	rm -rf "$our_cwd$tmp_auth_cert_directory_name"
	mkdir "$our_cwd$tmp_auth_cert_directory_name"
	cp "${name_string_for_authority_stuff}.conf" "$our_cwd$tmp_auth_cert_directory_name"
	pushd "$our_cwd$tmp_auth_cert_directory_name"
	pwd
	gener8_auth_cert
	popd
	pwd
	
}

copy_auth_cert_to_dirs(){
	echo "copy_auth_cert_to_dirs()"
	
	
	
	for((i=0; i<$total; i++)); 
	do
	
		mkdir "${complete_cert_directories_to_work_on[$i]}"
		cp -rf "$our_cwd$tmp_auth_cert_directory_name/"* "${complete_cert_directories_to_work_on[$i]}"
	done
}


export_host_certs_to_dirs(){
	echo "export_host_certs_to_dirs()"
	
	rm -rf "$our_cwd$tmp_host_cert_directory_name"
	mkdir "$our_cwd$tmp_host_cert_directory_name"
	
	pushd "$our_cwd$tmp_host_cert_directory_name"
	
	for((i=0; i<$total; i++)); 
	do
		cp -rf "$our_cwd$tmp_auth_cert_directory_name/"* "$our_cwd$tmp_host_cert_directory_name"
	
		module_name="${directories_to_work_on[$i]}"
		
		name_string_for_module="${name_string_for_authority_stuff}_${host_string_in_file_name}_${module_name}"


		echo "Module name: $module_name"
		echo "name_string_for_module: $name_string_for_module"
		
		pwd
		
		openssl genrsa -out "${name_string_for_module}.key" 2048

		openssl req -new -key "${name_string_for_module}.key" -out "${name_string_for_module}.csr" -config "${complete_cert_directories_to_work_on[$i]}/${name_string_for_module}.conf"


		openssl x509 -req -in "${name_string_for_module}.csr" -CA "${name_string_for_authority_stuff}.crt" -CAkey "${name_string_for_authority_stuff}.key" -CAcreateserial -out "${name_string_for_module}.crt" -days 825 -sha256 -extensions v3_req -extfile "${complete_cert_directories_to_work_on[$i]}/${name_string_for_module}.conf"
		
		curr_tg_dir="${complete_cert_directories_to_work_on[$i]}"
		
		echo "$curr_tg_dir"
		
		echo "Verifying this module certificate..."
		
		openssl verify -CAfile "${name_string_for_authority_stuff}.crt" "${name_string_for_module}.crt"
		
		openssl x509 -in "${name_string_for_module}.crt" -text -noout
		
		mv "$our_cwd$tmp_host_cert_directory_name"/* "${complete_cert_directories_to_work_on[$i]}"
		
	done
}

remv_cert_directories


cre8_auth_cert

copy_auth_cert_to_dirs

cre8_cert_module_name_files

export_host_certs_to_dirs


find $our_cwd -iname "*certs_and_pkeys*"

rm -rf "$our_cwd$tmp_host_cert_directory_name"

rm -rf "$our_cwd$tmp_auth_cert_directory_name"
