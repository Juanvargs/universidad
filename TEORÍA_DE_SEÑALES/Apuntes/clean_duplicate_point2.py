import json
from pathlib import Path

path = Path(r'c:/Universidad/TEORÍA_DE_SEÑALES/Apuntes/Binary_detection_lab.ipynb')
with path.open('r', encoding='utf-8') as f:
    data = json.load(f)

# Find consecutive duplicate 'Punto 2' code cells and remove the first one
new_cells = []
skip_next = False
for i, cell in enumerate(data['cells']):
    if skip_next:
        skip_next = False
        continue
    if cell['cell_type'] == 'code' and any('Punto 2: desarrollo de BinDetection_Lab (1).pdf' in line for line in cell.get('source', [])):
        # check next cell if also same marker
        if i + 1 < len(data['cells']):
            next_cell = data['cells'][i + 1]
            if next_cell['cell_type'] == 'code' and any('Punto 2: desarrollo de BinDetection_Lab (1).pdf' in line for line in next_cell.get('source', [])):
                # remove current duplicate and keep next cell
                skip_next = False
                continue
    new_cells.append(cell)

if len(new_cells) != len(data['cells']):
    data['cells'] = new_cells
    with path.open('w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print('Removed duplicate Punto 2 cell(s).')
else:
    print('No duplicate Punto 2 cells found.')
