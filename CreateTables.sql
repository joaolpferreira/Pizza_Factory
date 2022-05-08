CREATE TABLE atuador (
    atuador_nome VARCHAR(255) PRIMARY KEY
);

CREATE TABLE estado(
    estadoid SERIAL PRIMARY KEY,
    newstate VARCHAR(255)
);

CREATE TABLE mote(
    moteid INTEGER PRIMARY KEY
);

CREATE TABLE outputt(
    outputid INTEGER PRIMARY KEY,
    newstate_output VARCHAR(255) NOT NULL
);

CREATE TABLE sensor (
    sensor_nome VARCHAR(255) PRIMARY KEY,
    moteid INTEGER REFERENCES mote(moteid) NOT NULL
);

CREATE TABLE valor(
    valorid INTEGER PRIMARY KEY,
    valor_num VARCHAR(255) NOT NULL
);

CREATE TABLE inputt(
    inputid INTEGER PRIMARY KEY,
    comparador VARCHAR(255) NOT NULL,
    referencia INTEGER
);

CREATE TABLE regra(
    regranum INTEGER PRIMARY KEY
);

CREATE TABLE logico(
    logicos VARCHAR(5) NOT NULL,
    inputid INTEGER REFERENCES inputt(inputid), 
    regranum INTEGER REFERENCES regra(regranum),
    PRIMARY KEY(inputid)
);

CREATE TABLE regraoutput(
    regranum INTEGER REFERENCES regra(regranum),
    outputid INTEGER REFERENCES outputt(outputid),
    PRIMARY KEY (outputid)
);

CREATE TABLE sensoresvalor(
    sensor_nome VARCHAR(255) REFERENCES sensor(sensor_nome),
    valorid INTEGER REFERENCES valor(valorid),
    PRIMARY KEY (valorid)
);

CREATE TABLE outputatuador(
    atuador_nome INTEGER REFERENCES atuador(atuador_nome),
    outputid INTEGER REFERENCES outputt(outputid),
    PRIMARY KEY (atuador_nome, outputid)
);

CREATE TABLE atuadorestado(
    atuador_nome INTEGER REFERENCES atuador(atuador_nome),
    estadoid INTEGER REFERENCES estado(estadoid),
    PRIMARY KEY (estadoid)
);

CREATE TABLE sensoresinput(
    sensor_nome VARCHAR(255) REFERENCES sensor(sensor_nome),
    inputid INTEGER REFERENCES inputt(inputid),
    PRIMARY KEY (sensor_nome, inputid)
);