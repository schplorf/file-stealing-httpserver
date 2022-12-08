import requests

url = 'http://localhost:8080'
file_path = './my_file.txt'

with open(file_path, 'rb') as f:
    files = {'file': f}
    r = requests.post(url, files=files)

print(r.text)