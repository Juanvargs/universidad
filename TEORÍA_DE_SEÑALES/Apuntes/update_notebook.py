import json
from pathlib import Path
path = Path(r'c:/Universidad/TEORÍA_DE_SEÑALES/Apuntes/Binary_detection_lab.ipynb')
data = json.loads(path.read_text(encoding='utf-8'))

data['cells'][0]['source'] = [
    '# Punto 1 - MAP Threshold and Error Probability\n',
    '\n',
    'Este notebook está organizado por puntos. En esta primera sección resolvemos el primer enunciado del laboratorio:\n',
    '- derivar la regla MAP,\n',
    '- obtener la expresión analítica del umbral $\\gamma_{MAP}$,\n',
    '- implementar el detector MAP en Python,\n',
    '- barrer el umbral y calcular las probabilidades de falso alarma, omisión y error,\n',
    '- comparar el umbral numérico óptimo con el umbral analítico.\n',
    '\n',
    'Este cuaderno evita incluir abstract, resumen o conclusiones innecesarias. Solo se desarrollará lo necesario para cada punto.\n',
    '\n',
    'La segunda sección se agregará inmediatamente después para el archivo `BinDetection_Lab (1).pdf`.\n',
]

data['cells'].append({
    'cell_type': 'markdown',
    'metadata': {},
    'source': [
        '## Punto 2 - BinDetection_Lab (1).pdf\n',
        '\n',
        'En esta sección comenzaremos a trabajar el segundo enunciado del laboratorio.\n',
        'Primero leeremos los requerimientos del PDF y luego adaptaremos la implementación al nuevo problema.\n',
        '\n',
        'Este cuaderno sigue evitando incluir abstract, resumen o conclusiones innecesarias.\n',
        'Solo se añadirá el contenido necesario para resolver el segundo punto.\n',
    ],
})

path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')
