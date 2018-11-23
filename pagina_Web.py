# Instalar dependecias
#  pip3 install flask
#  pip3 install SQLAlchem
#  pip3 install request
#  pip3 install pylink

from flask import Flask, render_template, request, url_for, redirect, request, jsonify
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)
app.config['SQLACHEMY_DATABASE_URI'] = 'sqlite:///sqlite.db'
db = SQLAlchemy(app)


class PlacaLoRa(db.Model):
    __tablename__ = 'placa'
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    idCliente = db.Column(db.Integer)
    numPlaca = db.Column(db.Integer)
    medicao = db.Column(db.Integer)
    # talvez adicionar a data

    def __init__(self, idCliente, numPlaca, medicao):
        self.idCliente = idCliente
        self.numPlaca = numPlaca
        self.medicao = medicao


db.create_all()


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/cadastrar")
def cadastrar():
    return render_template("cadastrar.html")


@app.route("/cadastro", methods=['POST'])
def cadastro():
    if request.method == "POST":

        idCliente = request.form.get("idCliente")
        numPlaca = request.form.get("numPlaca")
        medicao = request.form.get("medicao")

        if request.get_json() != None:
            content = request.get_json()
            idCliente = content['idCliente']
            numPlaca = content['numPlaca']
            medicao = content['medicao']

        if idCliente and numPlaca and medicao:
            p = PlacaLoRa(idCliente, numPlaca, medicao)
            db.session.add(p)
            db.session.commit()

    return redirect(url_for("index"))


@app.route("/listar")
def listar():
    placaslora = PlacaLoRa.query.all()
    return render_template("listar.html", placaslora=placaslora)


if __name__ == '__main__':
    app.run(debug=True)
# como enviar o request
# POST http://localhost:5000/cadastro?idCliente=1&numPlaca=1&medicao=77&Cadastrar=1
