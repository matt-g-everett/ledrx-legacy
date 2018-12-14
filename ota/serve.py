import BaseHTTPServer, SimpleHTTPServer
import ssl
import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--port', dest='port', type= int,
    help= "Server Port", default= 8070)
args = parser.parse_args()

script_dir = os.path.dirname(os.path.realpath(__file__))
tls_dir = os.path.normpath(os.path.join(script_dir, '../tls'))
publish_dir = os.path.normpath(os.path.join(script_dir, '../publish'))

os.chdir(publish_dir)
httpd = BaseHTTPServer.HTTPServer(('', args.port),
        SimpleHTTPServer.SimpleHTTPRequestHandler)

httpd.socket = ssl.wrap_socket (httpd.socket,
    keyfile=os.path.join(tls_dir, "ca_key.pem"),
    certfile=os.path.join(tls_dir, "ca_cert.pem"), server_side=True)
print "Serving on 0.0.0.0 port ", args.port
httpd.serve_forever()
