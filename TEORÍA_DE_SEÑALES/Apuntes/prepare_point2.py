import json
from pathlib import Path
path = Path(r'c:/Universidad/TEORÍA_DE_SEÑALES/Apuntes/Binary_detection_lab.ipynb')
data = json.loads(path.read_text(encoding='utf-8'))

# Ensure the notebook ends with the Punto 2 markdown section
if not data['cells'] or data['cells'][-1]['cell_type'] != 'markdown':
    raise SystemExit('Expected last cell to be markdown')

last_markdown = data['cells'][-1]['source']
if not any('Punto 2 - BinDetection_Lab' in line for line in last_markdown):
    raise SystemExit('Expected Punto 2 markdown at the end')

# Add a code placeholder cell if not already present
if len(data['cells']) < 2 or data['cells'][-2]['cell_type'] != 'code' or data['cells'][-2]['source']:
    data['cells'].append({
        'cell_type': 'code',
        'execution_count': None,
        'metadata': {},
        'outputs': [],
        'source': [
            '# Punto 2: aquí iniciamos el desarrollo de BinDetection_Lab (1).pdf\n',
            '# Definiremos los parámetros de prior probabilities y calcularemos\n',
            '# el umbral MAP para cada valor de pi1.\n',
        ],
    })

path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')
