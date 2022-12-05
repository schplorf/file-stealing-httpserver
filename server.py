from http.server import HTTPServer, BaseHTTPRequestHandler
from io import BytesIO
import cgi

# This class defines a custom request handler that inherits from the base request handler
class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):

    # This method handles GET requests - override the base class method
    def do_POST(self):
        # Parse the request body as a form
        form = cgi.FieldStorage(
            fp=self.rfile, # rfile is a file-like object for reading the request body
            headers=self.headers, # headers is a dictionary of request headers
            environ={ # environ is a dictionary of environment variables
                'REQUEST_METHOD': 'POST',
                'CONTENT_TYPE': self.headers['Content-Type'],
            }
        )

        # Get the file data and file name from the form
        file_item = form['file']
        file_name = file_item.filename + '_uploaded'
        file_data = file_item.file.read()

        # Save the file to the current directory
        with open(file_name, 'wb') as f:
            f.write(file_data)

        # Send a response back to the client (200 = OK)
        self.send_response(200)
        self.end_headers()

        # Write the response body
        response = BytesIO()
        response.write(b'File saved to ' + file_name.encode())
        self.wfile.write(response.getvalue())

# Create an HTTP server and listen on port 8080
# Serve requests until process is killed
httpd = HTTPServer(('localhost', 8080), SimpleHTTPRequestHandler)
httpd.serve_forever()