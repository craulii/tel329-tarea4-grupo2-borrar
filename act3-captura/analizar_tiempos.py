# uso: python3 analizar_tiempos.py captura_coap.csv captura_mqtt.csv
# los csv se sacan de wireshark: filtro "coap || mqtt" y File > Export Packet Dissections > As CSV

import sys
import csv
import re

def leer_csv(nombre):
    filas = []
    with open(nombre, newline="") as f:
        for fila in csv.DictReader(f):
            filas.append(fila)
    return filas

def tiempos_coap(filas):
    envios = {}
    respuestas = {}
    for fila in filas:
        info = fila["Info"]
        t = float(fila["Time"])
        m = re.search(r"MID:\s*(\d+)", info)
        if m is None:
            continue
        mid = m.group(1)
        if info.startswith("CON"):
            clave = (fila["Source"], mid)
            if clave not in envios:
                envios[clave] = t
        elif info.startswith("ACK"):
            clave = (fila["Destination"], mid)
            respuestas[clave] = t
    return emparejar(envios, respuestas)

def tiempos_mqtt(filas):
    envios = {}
    respuestas = {}
    for fila in filas:
        info = fila["Info"]
        t = float(fila["Time"])
        m = re.search(r"Publish Message \(id=(\d+)\)", info)
        if m is not None:
            clave = (fila["Source"], m.group(1))
            if clave not in envios:
                envios[clave] = t
            continue
        m = re.search(r"Publish Ack \(id=(\d+)\)", info)
        if m is not None:
            clave = (fila["Destination"], m.group(1))
            respuestas[clave] = t
    return emparejar(envios, respuestas)

def emparejar(envios, respuestas):
    resultado = []
    for clave in envios:
        if clave in respuestas and respuestas[clave] >= envios[clave]:
            rtt = (respuestas[clave] - envios[clave]) * 1000
            resultado.append((clave[0], clave[1], rtt))
    return resultado

def resumen(nombre, datos):
    if len(datos) == 0:
        print(nombre, "sin datos")
        return
    rtts = [d[2] for d in datos]
    print(nombre, "muestras:", len(rtts), "min:", round(min(rtts), 1), "ms",
          "prom:", round(sum(rtts) / len(rtts), 1), "ms", "max:", round(max(rtts), 1), "ms")
    nodos = {}
    for nodo, mid, rtt in datos:
        if nodo not in nodos:
            nodos[nodo] = []
        nodos[nodo].append(rtt)
    for nodo in sorted(nodos):
        lista = nodos[nodo]
        print("  ", nodo, "n =", len(lista), "prom =", round(sum(lista) / len(lista), 1), "ms")

datos_coap = tiempos_coap(leer_csv(sys.argv[1]))
datos_mqtt = tiempos_mqtt(leer_csv(sys.argv[2]))

resumen("coap", datos_coap)
resumen("mqtt", datos_mqtt)

with open("tiempos.csv", "w") as f:
    f.write("protocolo,nodo,id,rtt_ms\n")
    for nodo, mid, rtt in datos_coap:
        f.write(f"coap,{nodo},{mid},{round(rtt, 1)}\n")
    for nodo, mid, rtt in datos_mqtt:
        f.write(f"mqtt,{nodo},{mid},{round(rtt, 1)}\n")
