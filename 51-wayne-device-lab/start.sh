#!/bin/bash
echo "WAYNE TOOL - DEVICE LAB - Iniciando..."
echo "Abrindo na porta 8000..."
echo "Depois, abra no navegador: localhost:8000/51-wayne-device-lab/"
python3 -m http.server 8000
