#include <WinSock2.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " ip port" << std::endl;
    return -1;
  }
  
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(argv[2]));
  addr.sin_addr.s_addr = inet_addr(argv[1]);

  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != NO_ERROR) {
    wprintf(L"WSAStartup function failed with error: %d\n", iResult);
    return 1;
  }

  SOCKET h = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  u_long nonblock = 1; /* non-blocking enable */
  if (NO_ERROR != ioctlsocket(h, FIONBIO, &nonblock)) {
    cerr << "Failed to invoke ioctlsocket!" << endl;
    closesocket(h);
    WSACleanup();

    return -1;
  }

  int ret = connect(h, (sockaddr *)&addr, sizeof(addr));
  if (ret != NO_ERROR) {
    fd_set rds;
    fd_set wds;
    fd_set eds;
    
    FD_ZERO(&rds);
    FD_ZERO(&wds);
    FD_ZERO(&eds);
    
    FD_SET(h, &rds);
    FD_SET(h, &wds);
    FD_SET(h, &eds);

    int n = select(int(h) + 1, &rds, &wds, &eds, NULL);
    if (n < 0) {
      cerr << "Failed to invoke select: " << WSAGetLastError() << endl;
      return -1;
    } else if (n > 0) {
      if (FD_ISSET(h, &rds)) {
        cout << "Readable: should not run here " << endl;
      }
      if (FD_ISSET(h, &wds)) {
        cout << "Writable: Connection success " << endl;
      }
      if (FD_ISSET(h, &eds)) {
        int error_code = 0;
        int len = sizeof(error_code);
        getsockopt(h, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error_code), &len);
        cout << "Exception occurred, Reason: " << error_code  << endl;
      }
    }
  }

  closesocket(h);
  WSACleanup();
  return 0;
}

