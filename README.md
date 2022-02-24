# HTTP-Caching-Proxy

In this project, I realize an http proxy – a server whose job it is to forward requests to the origin server on behalf of the client. The proxy will cache responses, and, when appropriate, respond with the cached copy of a resource rather than re-fetching it. I manage the cache via parsing cache-control and expire header, revalidate with Etag and Last-Modified header. The proxy can also identify malformed requests and corrupted responses and return 400 Bad Request and 502 Bad Gateway messages to the client.

