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
        print(bcolors.FAIL + '{} [KO] {} \t expected: {} yours {}'.format(testcase[:3], testcase, expectedCode, http_response.status) + bcolors.ENDC)
    else:
        print(bcolors.OKGREEN + '{} [OK] {}'.format(testcase[:3], http_response.status) + bcolors.ENDC)
    # return (http_response.read().decode('utf-8'))

def stress_test(n):
        print(bcolors.OKBLUE + 'Stress Testing With {} requests'.format(n) + bcolors.ENDC)
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
    tester()
    end = time.time()
    print('Time: {}s'.format(str(end - start)[:4]))


def test_400():
    print(bcolors.OKBLUE + 'Testing 400\'s' + bcolors.ENDC)

    # multiple spaces
    # request_header = 'GET  /  HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    # compare(request_header, 400, '1. multiple spaces')

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

    # no host
    request_header = 'GET / HTTP/1.1\r\n\r\n'
    compare(request_header, 400, '6. no host')

    # multiple host
    request_header = 'GET / HTTP/1.1\r\nHost: naver.com\r\nHost: hyeyoo.com\r\n\r\n'
    compare(request_header, 400, '7. multiple host')

    # multiple host 2
    request_header = 'GET / HTTP/1.1\r\nHost: {}\r\nHost: {}\r\n\r\n'.format(host, host)
    compare(request_header, 400, '8. multiple host 2')

    # invalid field value in host
    request_header = 'GET / HTTP/1.1\r\nHost: hyeyoo@hyeyoo.com\r\n\r\n'
    compare(request_header, 400, '9. invalid field value in host')

    # invalid content length
    length = '-1'
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: {}\r\n\r\n'.format(host, length)
    compare(request_header, 400, '10. invalid content length')

    # invalid content length 3
    length = 'NOTDIGIT'
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: {}\r\n\r\n'.format(host, length)
    compare(request_header, 400, '11. invalid content length 3')

    # Content-Length with Transfer-Encoding
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: 10000\r\nTransfer-Encoding: chunked\r\n\r\n0'.format(host)
    compare(request_header, 400, '12. Content-Length with Transfer-Encoding')

    # multiple Content-Length differing size
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: 1\r\nContent-Length: 0\r\n\r\n'.format(host)
    compare(request_header, 400, '13. multiple Content-Length differing size')

    # Some stupid stuff
    request_header = 'Host:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '14. Some stupid stuff')

    # random header with no uri
    request_header = 'GET HTTP/1.1\r\ns:{}\r\n\r\n'.format(host)
    compare(request_header, 400, '15. random header')

def test_200():
    print(bcolors.OKBLUE + 'Testing 200\'s' + bcolors.ENDC)

    # valid field value in host 2
    request_header = 'GET / HTTP/1.1\r\nHost: hyeyoo.com:8080\r\n\r\n'
    compare(request_header, 200, '1. valid field value in host 2')

    # GET CGI request
    request_header = 'GET /post/submission.php?name=bob&email=test@codam.nl&message=HALLOO HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '2. Get CGI request')

    # GET allowed in root
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '3. GET allowed in root')

    # Method is allowed
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '4. Method is allowed')

    # POST allowed in post directory
    request_header = 'POST /post/submission.php HTTP/1.1\r\nHost:{}\r\nContent-Length: 84\r\n\r\nname=Bobbie&email=root%40root.com&message=Wie+Ben+jij+dan&contact_submitted=submit\r\n'.format(host)
    compare(request_header, 200, '5. POST allowed in post directory')

    # POST with encoding
    request_header = 'POST /post/submission.php HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nHost:{}\r\nTransfer-Encoding: chunked\r\n\r\na\r\nname=Hoi&m\r\na\r\nessage=Bob\r\na\r\nbieee&cont\r\na\r\nact_submit\r\na\r\nted=submit\r\n0\r\n\r\n'.format(host)
    compare(request_header, 200, '6. POST with encoding')

    # GET with CGI
    request_header = 'GET /post/submission.php?name=Bobbie&messageThisIsGetWithCGI HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 200, '7. GET with CGI')

    # POST with encoding with very long length
    request_header = 'POST /post/submission.php HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nHost:{}\r\nTransfer-Encoding: chunked\r\n\r\n{}'.format(host, "a\r\nname=HALLO\r\na\r\n_DIT_ZIT_I\r\na\r\nN_DE_DEFIN\r\na\r\nE_EN_IS_EE\r\na\r\nN_STRING&m\r\na\r\nessage=IK_\r\na\r\nMOET_HIER_\r\na\r\nRANDOM_DIN\r\na\r\nGEN_TYPEN_\r\na\r\nOM_TE_TEST\r\na\r\nEN_OF_HET_\r\na\r\nWERKT&cont\r\na\r\nact_submit\r\na\r\nted=submit\r\n0\r\n\r\n")
    compare(request_header, 200, '8. POST with encoding with very long length')

def test_xxx():
    print(bcolors.OKBLUE + 'Testing xxx\'s' + bcolors.ENDC)

    # send invalid http version
    request_header = 'GET / HTTP/1.2\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 505, '1. send invalid http version')

    # send invalid method
    request_header = 'GETT / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 405, '2. send invalid method')

    # redirection to http://www.google.com if host is 127.0.0.1:8080
    request_header = 'GET /redirect/ HTTP/1.1\r\nHost:127.0.0.1:{}\r\n\r\n'.format(config.SERVER_PORT)
    compare(request_header, 301, '3. redirection to http://www.google.com if host is 127.0.0.1:8080')

    # redirection to https://www.codam.nl if host is 127.0.0.1:8080
    request_header = 'GET /redirect/ HTTP/1.1\r\nHost:localhost:{}\r\n\r\n'.format(config.SERVER_PORT)
    compare(request_header, 307, '4. redirection to https://www.codam.nl if host is localhost:8080')

    # invalid content length 2
    length = '100000000000000000000000'
    request_header = 'GET / HTTP/1.1\r\nHost:{}\r\nContent-Length: {}\r\n\r\n'.format(host, length)
    compare(request_header, 431, '5. invalid content length 2')

    # Post not allowed in root
    request_header = 'POST / HTTP/1.1\r\nHost:{}\r\nContent-Length: 23\r\n\r\nHallo Dit is een test\r\n'.format(host)
    compare(request_header, 405, '6. POST not allowed in root')

    # PUT not allowed in post directory
    request_header = 'PUT /post/ HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 405, '7. PUT not allowed in post directory')

    # Method is not allowed
    request_header = 'ROWAN / HTTP/1.1\r\nHost:{}\r\n\r\n'.format(host)
    compare(request_header, 405, '8. Method is not allowed')

    # empty request
    request_header = 'get / http/1.1\r\nHost: localhost.com:8080\r\n\r\n'
    compare(request_header, 405, '9. empty request')

def tester():
    # test_400()
    test_200()
    # test_xxx()
    # stress_test(2048)


if __name__ == '__main__':
    run()