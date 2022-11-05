import sys
import time
sys.path.append('../')
sys.path.append('../lib')
from send_request import send_request
import config
import socket
from http.client import HTTPResponse

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# test rfc7230 section 3.3.1: Request Line

host = config.SERVER_ADDR + ':' + str(config.SERVER_PORT)

def compare(request_header, expectedCode, testcase):
    http_response = send_request(request_header)
    if http_response.status != expectedCode:
        print(bcolors.FAIL + '[KO] {} \t expected: {} yours {}'.format(testcase, expectedCode, http_response.status) + bcolors.ENDC)
    else:
        print(bcolors.OKGREEN + '[OK] {}'.format(http_response.status) + bcolors.ENDC)

def stress_test(n):
        # Stress Testing
        start = time.time()
        went_wrong = False
        request_header = 'GET / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
        for x in range(n):
            http_response = send_request(request_header)
            if http_response.status != 200:
                went_wrong = True
        end = time.time()
        if (went_wrong == False):
            print(bcolors.OKGREEN + '[OK] {} requests took {} seconds'.format(n, str(end - start)[:4]) + bcolors.ENDC)
        else:
            print(bcolors.FAIL + '[FAIL] {} requests took {} seconds'.format(n, str(end - start)[:4]) + bcolors.ENDC)


def run():
    start = time.time()
    print('testing {}...'.format(__file__))

    # multiple spaces
    request_header = 'GET  /  HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '1. multiple spaces')


    # Takes long so commmented out
    # # too long URI
    # request_header = 'GET  /' + 'a' * (config.MAX_URI_LENGTH + 1) + ' HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    # compare(request_header, 400, '2. too long URI')

    # no space between method and uri
    request_header = 'GET/ HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '3. no space between method and uri')

    # no space between uri and version
    request_header = 'GET /HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '4. no space between uri and version')

    # mising uri in request line
    request_header = 'GET HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '5. mising uri in request line')

    # send invalid http version
    request_header = 'GET / HTTP/1.2\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 505, '6. send invalid http version')

    # send invalid method
    request_header = 'GETT / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 405, '7. send invalid method')

    # no host
    request_header = 'GET / HTTP/1.1\r\n\r\n'
    compare(request_header, 400, '8. no host')

      # multiple host
    request_header = 'GET / HTTP/1.1\r\nHost: naver.com\r\nHost: hyeyoo.com\r\n\r\n'
    compare(request_header, 400, '9. multiple host')

    # multiple host 2
    request_header = 'GET / HTTP/1.1\r\nHost: {}\r\nHost: {}\r\n\r\n'.format(host, host)
    compare(request_header, 400, '10. multiple host 2')

    # invalid field value in host
    request_header = 'GET / HTTP/1.1\r\nHost: hyeyoo@hyeyoo.com\r\n\r\n'
    compare(request_header, 400, '11. invalid field value in host')

    # valid field value in host 2
    request_header = 'GET / HTTP/1.1\r\nHost: hyeyoo.com:8080\r\n\r\n'
    compare(request_header, 200, '12. valid field value in host 2')

    # invalid content length
    length = '-1'
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: {}\r\n\r\n'.format(host, length)
    compare(request_header, 400, '13. invalid content length')

    length = '100000000000000000000000'
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: {}\r\n\r\n'.format(host, length)
    compare(request_header, 431, '14. invalid content length 2')

    length = 'NOTDIGIT'
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: {}\r\n\r\n'.format(host, length)
    compare(request_header, 400, '15. invalid content length 3')

    # Content-Length with Transfer-Encoding
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: 10000\r\nTransfer-Encoding: chunked\r\n\r\n0'.format(host)
    compare(request_header, 400, '16. Content-Length with Transfer-Encoding')

    # multiple Content-Length differing size
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: 1\r\nContent-Length: 0\r\n\r\n'.format(host)
    compare(request_header, 400, '17. multiple Content-Length differing size')

    # redirection to http://www.google.com if host is 127.0.0.1:8080
    request_header = 'GET /redirect/ HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 301, '18. redirection to http://www.google.com if host is 127.0.0.1:8080')

    # redirection to https://www.codam.nl if host is 127.0.0.1:8080
    request_header = 'GET /redirect/ HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 307, '19. redirection to https://www.codam.nl if host is localhost:8080')

    # Get CGI request
    request_header = 'GET /post/submission.php?name=bob&email=test@codam.nl&message=HALLOO HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '20. Get CGI request')

    # Post not allowed in root
    request_header = 'POST / HTTP/1.1\r\nHost:{}\r\nContent-Length: 23\r\n\r\nHallo Dit is een test\r\n'.format(host)
    compare(request_header, 405, '21. POST not allowed in root')

    # GET allowed in root
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '22. GET allowed in root')

    # POST allowed in post directory
    request_header = 'POST /post/submission.php HTTP/1.1\r\nHost:{}\r\nContent-Length: 84\r\n\r\nname=Bobbie&email=root%40root.com&message=Wie+Ben+jij+dan&contact_submitted=submit\r\n'.format(host)
    compare(request_header, 200, '23. POST allowed in post directory')

    # PUT not allowed in post directory
    request_header = 'PUT /post/ HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 405, '24. PUT not allowed in post directory')

    # Method is allowed
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '25. Method is allowed')

    # Some stupid stuff
    request_header = 'Host:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '26. Some stupid stuff')












    end = time.time()
    print('Time: {}s'.format(str(end - start)[:4]))

if __name__ == '__main__':
    run()
