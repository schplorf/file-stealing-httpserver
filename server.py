from http.server import HTTPServer, BaseHTTPRequestHandler
from io import BytesIO
import cgi

# This class defines a custom request handler that inherits from the base request handler
class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    # This method handles GET requests - override the base class method
    def do_POST(self):
        print("POST request received")
        # Parse the request body as a form
        form = cgi.FieldStorage(
            fp=self.rfile, # rfile is a file-like object for reading the request body
            headers=self.headers, # headers is a dictionary of request headers
            environ={ # environ is a dictionary of environment variables
                'REQUEST_METHOD': 'POST',
                'CONTENT_TYPE': self.headers['Content-Type'],
            }
        )
        #print("Form data: ", form)
        # Get the IP of the client
        client_ip = self.client_address[0]

        # Get the file data and file name from the form
        file_item = form['file']
        file_name = client_ip + '_' + file_item.filename
        file_data = file_item.file.read()

        # Save the file to the current directory
        # Dont save if the file already exists
        try:
            with open(file_name, 'xb') as f:
                f.write(file_data)
                print("File saved as ", file_name)
        except FileExistsError:
            print("File already exists")
        
        print("Done processing POST request")

# Create an HTTP server and listen on port 8080
# Serve requests until process is killed
# Allow any IP to connect
httpd = HTTPServer(('', 8080), SimpleHTTPRequestHandler)
httpd.serve_forever()