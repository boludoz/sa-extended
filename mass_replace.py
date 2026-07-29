import os
import sys
import csv
import re
import argparse

def mass_replace(csv_path, target_dir, file_extensions=None, dry_run=False, reverse=False):
    """
    Lee un archivo CSV con pares de (buscar, reemplazar) y realiza el reemplazo
    masivo por coincidencia exacta de palabra (\bword\b) en todas las subcarpetas.
    
    Si reverse=True, intercambia el orden (reemplaza Columna 2 -> Columna 1).
    """
    if not os.path.isfile(csv_path):
        print(f"❌ Error: El archivo CSV '{csv_path}' no existe.")
        return

    replacements = []

    # Intentar abrir con encoding utf-8-sig (para manejar BOM de Excel)
    with open(csv_path, mode='r', encoding='utf-8-sig', newline='') as f:
        reader = csv.reader(f)
        rows = [row for row in reader if row]

    if not rows:
        print("❌ Error: El archivo CSV está vacío.")
        return

    # Detectar formato del CSV
    if len(rows) == 2 and len(rows[0]) > 2:
        # Formato Horizontal (Fila 1 = Buscados, Fila 2 = Reemplazos)
        for search_term, replace_term in zip(rows[0], rows[1]):
            search_term = search_term.strip()
            replace_term = replace_term.strip()
            if search_term and replace_term:
                if reverse:
                    replacements.append((replace_term, search_term))
                else:
                    replacements.append((search_term, replace_term))
    else:
        # Formato Vertical (Columna 1 = Buscar, Columna 2 = Reemplazar)
        for row in rows:
            if len(row) >= 2:
                search_term = row[0].strip()
                replace_term = row[1].strip()
                if search_term and replace_term:
                    if reverse:
                        replacements.append((replace_term, search_term))
                    else:
                        replacements.append((search_term, replace_term))

    if not replacements:
        print("❌ No se encontraron pares de reemplazo válidos en el CSV.")
        return

    direction_str = " (Modo INVERSO: Columna 2 ➔ Columna 1)" if reverse else " (Modo NORMAL: Columna 1 ➔ Columna 2)"
    print(f"📋 Se cargaron {len(replacements)} pares de reemplazo del CSV.{direction_str}")
    for orig, rep in replacements[:5]:
        print(f"   • '{orig}' ➔ '{rep}'")
    if len(replacements) > 5:
        print(f"   ... ({len(replacements) - 5} pares más)")

    # Compilar expresiones regulares con \b para coincidencia exacta palabra por palabra
    regex_replacements = []
    for search_term, replace_term in replacements:
        pattern = r'\b' + re.escape(search_term) + r'\b'
        regex = re.compile(pattern)
        regex_replacements.append((regex, replace_term, search_term))

    total_files_scanned = 0
    total_files_modified = 0
    total_replacements_made = 0

    print(f"\n🔍 Buscando en directorio: '{os.path.abspath(target_dir)}'...")

    for root, dirs, files in os.walk(target_dir):
        # Omitir carpetas ocultas o de compilación masiva
        dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ('build', 'node_modules', '__pycache__', 'out')]

        for file in files:
            if file_extensions:
                if not any(file.lower().endswith(ext.lower()) for ext in file_extensions):
                    continue

            file_path = os.path.join(root, file)
            total_files_scanned += 1

            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                new_content = content
                file_changes = 0

                for regex, replace_term, orig_term in regex_replacements:
                    new_content, count = regex.subn(replace_term, new_content)
                    if count > 0:
                        file_changes += count

                if file_changes > 0:
                    total_files_modified += 1
                    total_replacements_made += file_changes
                    mode_str = "SIMULACIÓN" if dry_run else "MODIFICADO"
                    print(f" [{mode_str}] {file_path} ({file_changes} cambios)")

                    if not dry_run:
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(new_content)

            except Exception as e:
                print(f"⚠️ Error procesando {file_path}: {e}")

    print("\n================ RESUMEN DE EJECUCIÓN ================")
    print(f" Modo:                   {'INVERSO (-r)' if reverse else 'NORMAL'}")
    print(f" Archivos analizados:    {total_files_scanned}")
    print(f" Archivos modificados:   {total_files_modified}")
    print(f" Total de reemplazos:    {total_replacements_made}")
    if dry_run:
        print(" (Modo simulación activo: no se guardaron cambios en disco)")
    print("======================================================")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Reemplazo masivo palabra por palabra desde un CSV.")
    parser.add_argument("csv_file", help="Ruta al archivo CSV con los términos.")
    parser.add_argument("--dir", default=".", help="Directorio raíz a procesar (por defecto el actual).")
    parser.add_argument("--ext", nargs="*", help="Filtro de extensiones (ejemplo: .cpp .h .txt).")
    parser.add_argument("--dry-run", action="store_true", help="Simular sin modificar archivos en disco.")
    parser.add_argument("-r", "--reverse", action="store_true", help="Reemplazar a la inversa (Columna 2 -> Columna 1).")

    args = parser.parse_args()
    mass_replace(args.csv_file, args.dir, args.ext, args.dry_run, args.reverse)
