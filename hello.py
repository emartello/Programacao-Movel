#Para rodar
# FLASK_APP=hello.py export FLASK_ENV=development
# FLASK_APP=hello.py flask run

from flask import Flask
app = Flask(__name__)

@app.route("/")
def hello():
    return "Hello World!"
