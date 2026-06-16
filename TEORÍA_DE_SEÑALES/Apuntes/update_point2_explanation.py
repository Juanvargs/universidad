import json
from pathlib import Path

path = Path(r'c:/Universidad/TEORÍA_DE_SEÑALES/Apuntes/Binary_detection_lab.ipynb')
with path.open('r', encoding='utf-8') as f:
    data = json.load(f)

# Find all Punto 2 code cells
point2_indices = [i for i, cell in enumerate(data['cells']) if cell['cell_type'] == 'code' and any('Punto 2: desarrollo de BinDetection_Lab (1).pdf' in line for line in cell.get('source', []))]
if not point2_indices:
    raise SystemExit('No Punto 2 code cells found')

# Keep only the first Punto 2 code cell and remove duplicates
keep_index = point2_indices[0]
for i in reversed(point2_indices[1:]):
    del data['cells'][i]

# Insert a markdown explanation cell before the Punto 2 code block if not already present
insert_index = keep_index
if insert_index == 0 or data['cells'][insert_index - 1]['cell_type'] != 'markdown' or '## Punto 2' not in ''.join(data['cells'][insert_index - 1].get('source', [])):
    explanation_cell = {
        'cell_type': 'markdown',
        'metadata': {},
        'source': [
            '## 5. Punto 2: efecto de las probabilidades a priori\n',
            '\n',
            'En este punto usamos la misma configuración de detección binaria con ruido AWGN, pero variamos las probabilidades a priori de la hipótesis $H_1$.\n',
            'Para cada valor de $\pi_1 \in \{0.1,0.3,0.5,0.7,0.9\}$ calculamos el umbral MAP, y luego evaluamos las probabilidades de error de falso ingreso y omisión.\n',
            '\n',
            'La expresión analítica del umbral MAP es:\n',
            '\\[\gamma_{MAP} = \frac{a_0 + a_1}{2} + \frac{\sigma_{\eta}^2}{a_1 - a_0} \ln\left(\frac{\pi_0}{\pi_1}\right).\\]\n',
            '\n',
            'Este umbral se desplaza hacia la hipótesis menos probable porque el término logarítmico cambia de signo según cuál prior sea mayor.\n',
            'También veremos cómo cambian $P_{FA}$, $P_M$ y $P_e$ cuando el detector favorece una u otra hipótesis.\n',
        ],
    }
    data['cells'].insert(insert_index, explanation_cell)

# Update the existing Punto 2 markdown cell for clearer explanation after the code
for cell in data['cells']:
    if cell['cell_type'] == 'markdown' and any('Punto 2 - BinDetection_Lab' in line for line in cell.get('source', [])):
        cell['source'] = [
            '## Punto 2 - BinDetection_Lab (1).pdf\n',
            '\n',
            'En esta sección se completan los cálculos y las gráficas que pide el segundo enunciado.\n',
            '- Calculamos $\gamma_{MAP}$ para cada valor de $\pi_1$.\n',
            '- Calculamos $P_{FA}$, $P_M$ y $P_e$ en el umbral MAP correspondiente.\n',
            '- Graficamos $\gamma_{MAP}$ vs. $\pi_1$ y las probabilidades de error vs. $\pi_1$.\n',
            '\n',
            'Esta explicación acompaña la implementación y hace explícita la relación entre prior probabilities y la posición del umbral.\n',
        ]
        break

with path.open('w', encoding='utf-8') as f:
    json.dump(data, f, ensure_ascii=False, indent=2)
