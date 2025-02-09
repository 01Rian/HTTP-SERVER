#include <stdio.h>
#include <string.h>
#include "http_parser.h"

// Este parser é bastante simples e espera receber uma linha de requisição
// no formato: "GET /caminho HTTP/1.1"
int parse_http_request(const char *request, char *method, size_t method_size, char *path, size_t path_size)
{
  
  if (!request || !method || !path) {
    return -1;
  }

  // Utiliza sscanf para extrair o método e o caminho
  int ret = sscanf(request, "%7s %1023s", method, path);

  if (ret != 2) {
    return -1;
  }
  
  return 0;
}