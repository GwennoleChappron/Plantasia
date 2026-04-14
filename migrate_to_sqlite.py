#!/usr/bin/env python3
"""
migrate_to_sqlite.py - Version Définitive (Avec gestion des Thèmes UI)
Usage : python3 migrate_to_sqlite.py
"""

import sqlite3
import json
import pathlib
import sys

# ─────────────────────────────────────────────────────────────
# Thèmes et Couleurs
# ─────────────────────────────────────────────────────────────
THEME = {
    "PlantGreen":    (0.20, 0.75, 0.40, 1.0),
    "PlantMid":      (0.15, 0.55, 0.30, 1.0),
    "PlantDark":     (0.10, 0.35, 0.18, 1.0),
    "Amber":         (0.93, 0.70, 0.13, 1.0),
    "WarningOrange": (1.00, 0.60, 0.10, 1.0),
    "DangerRed":     (0.85, 0.20, 0.20, 1.0),
    "InfoBlue":      (0.30, 0.60, 0.90, 1.0),
    "TextMuted":     (0.40, 0.50, 0.40, 1.0),
    "TextSecondary": (0.55, 0.68, 0.55, 1.0),
    "Violet":        (0.60, 0.30, 0.80, 1.0),
    "Floraison":     (0.85, 0.45, 0.60, 1.0),
}

# Dictionnaires de base pour l'UI
ENUM_METADATA = {
    "NiveauDifficulte": [
        ("FACILE", "Facile", "Pour débutants", "PlantGreen", 1), 
        ("MOYEN", "Moyen", "Surveillance régulière", "Amber", 2),
        ("DIFFICILE", "Difficile", "Conditions précises", "DangerRed", 3)
    ],
    "TypePlante": [
        ("AROMATIQUE", "Aromatique", "", "PlantGreen", 1), 
        ("ORNEMENTALE", "Ornementale", "", "Violet", 2),
        ("FRUITIER", "Fruitier", "", "WarningOrange", 3), 
        ("LEGUMINEUSE", "Légumineuse", "", "PlantMid", 4),
        ("VIVACE", "Vivace", "", "Amber", 5)
    ],
    "ExpositionSoleil": [
        ("PLEIN_SOLEIL", "Plein soleil", "", "Amber", 1), 
        ("MI_OMBRE", "Mi-ombre", "", "PlantGreen", 2),
        ("OMBRE", "Ombre", "", "InfoBlue", 3)
    ],
    "ExpositionVent": [
        ("MODERE", "Vent modéré", "", "TextSecondary", 1), 
        ("ABRITE", "Abrité", "", "InfoBlue", 2)
    ],
    "Rusticite": [
        ("RUSTIQUE", "Rustique", "", "PlantGreen", 1), 
        ("SEMI_RUSTIQUE", "Semi-rustique", "", "Amber", 2),
        ("FRAGILE", "Fragile", "", "DangerRed", 3)
    ],
    "VitesseCroissance": [
        ("LENTE", "Lente", "", "TextMuted", 1), 
        ("MODEREE", "Modérée", "", "PlantGreen", 2),
        ("RAPIDE", "Rapide", "", "Amber", 3), 
        ("TRES_RAPIDE", "Très rapide", "", "WarningOrange", 4)
    ],
    "ToleranceSecheresse": [
        ("EXCELLENTE", "Excellente", "", "PlantGreen", 1), 
        ("BONNE", "Bonne", "", "PlantMid", 2),
        ("MOYENNE", "Moyenne", "", "Amber", 3), 
        ("FAIBLE", "Faible", "", "WarningOrange", 4),
        ("TRES_FAIBLE", "Très faible", "", "DangerRed", 5)
    ],
    "SensibiliteEau": [
        ("TRES_ELEVEE", "Très élevée", "", "DangerRed", 1), 
        ("ELEVEE", "Élevée", "", "WarningOrange", 2),
        ("MOYENNE", "Moyenne", "", "Amber", 3), 
        ("FAIBLE", "Faible", "", "PlantGreen", 4)
    ],
    "TypeFeuillage": [
        ("PERSISTANT", "Persistant", "", "PlantGreen", 1), 
        ("CADUC", "Caduc", "", "WarningOrange", 2),
        ("SEMI_PERSISTANT", "Semi-persistant", "", "Amber", 3)
    ]
}

# ─────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────
def safe_str(data, key, default=""): return str(data.get(key, default) or default)
def safe_int(data, key, default=0): 
    try: return int(data.get(key, default) or default)
    except: return default
def safe_bool(data, key, default=False): return bool(data.get(key, default))
def load_json(path):
    with open(path, encoding="utf-8") as f: return json.load(f)

# ─────────────────────────────────────────────────────────────
# Migration
# ─────────────────────────────────────────────────────────────
def migrate(json_dir: pathlib.Path, db_path: pathlib.Path):
    print(f"📦 Création de la base : {db_path}")
    db_path.unlink(missing_ok=True)
    con = sqlite3.connect(db_path)
    con.execute("PRAGMA foreign_keys = ON")

    # 1. SCHÉMA COMPLET
    schema_sql = """
    -- =========================================================
    -- 0. TABLE DES THÈMES (Couleurs UI)
    -- =========================================================
    CREATE TABLE ref_theme (
        code TEXT PRIMARY KEY, 
        r REAL NOT NULL, 
        g REAL NOT NULL, 
        b REAL NOT NULL, 
        a REAL NOT NULL
    );

    -- =========================================================
    -- 1. TABLES DE RÉFÉRENCE (DICTIONNAIRES pointant vers ref_theme)
    -- =========================================================
    CREATE TABLE ref_difficulte (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_type_plante (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_exposition_soleil (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_exposition_vent (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_rusticite (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_vitesse_croissance (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_tolerance_secheresse (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_sensibilite_eau (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);
    CREATE TABLE ref_feuillage (code TEXT PRIMARY KEY, label_ui TEXT, description TEXT, theme_code TEXT REFERENCES ref_theme(code) ON UPDATE CASCADE, sort_order INTEGER);

    -- =========================================================
    -- 2. TABLES RÉFÉRENTIELLES COMPLEXES
    -- =========================================================
    CREATE TABLE sols (
        type_sol TEXT PRIMARY KEY, texture TEXT, drainage TEXT, retention_eau TEXT, 
        richesse TEXT, ph_min REAL, ph_max REAL, utilisation TEXT, cec TEXT, 
        aeration TEXT, densite TEXT, tampon_ph TEXT, mineralisation TEXT, 
        vie_microbienne TEXT, frequence_renouvellement TEXT, compatibilite_calcaire TEXT
    );
    CREATE TABLE sols_composition (sol_id TEXT REFERENCES sols(type_sol) ON DELETE CASCADE, composant TEXT, pourcentage REAL, PRIMARY KEY(sol_id, composant));
    CREATE TABLE sols_risques (sol_id TEXT REFERENCES sols(type_sol) ON DELETE CASCADE, risque TEXT, PRIMARY KEY(sol_id, risque));

    CREATE TABLE racines (
        type_racinaire TEXT PRIMARY KEY, profondeur TEXT, largeur TEXT, 
        volume_min INTEGER, volume_max INTEGER, forme_pot TEXT, drainage TEXT, 
        frequence_rempotage TEXT, sensibilite_rempotage TEXT
    );
    CREATE TABLE racines_materiaux (racine_id TEXT REFERENCES racines(type_racinaire) ON DELETE CASCADE, materiau TEXT, PRIMARY KEY(racine_id, materiau));

    CREATE TABLE boutures (
        type_bouture TEXT PRIMARY KEY, difficulte TEXT, type_tige TEXT, 
        longueur_min INTEGER, longueur_max INTEGER, substrat TEXT, hormone TEXT, 
        humidite TEXT, temp_min INTEGER, temp_max INTEGER, 
        enracinement_min INTEGER, enracinement_max INTEGER, taux_reussite TEXT
    );
    CREATE TABLE boutures_etapes (bouture_id TEXT REFERENCES boutures(type_bouture) ON DELETE CASCADE, ordre INTEGER, etape TEXT, PRIMARY KEY(bouture_id, ordre));
    CREATE TABLE boutures_periodes (bouture_id TEXT REFERENCES boutures(type_bouture) ON DELETE CASCADE, periode TEXT, PRIMARY KEY(bouture_id, periode));

    -- =========================================================
    -- 3. TABLE PRINCIPALE : PLANTES
    -- =========================================================
    CREATE TABLE plantes (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        nom TEXT NOT NULL UNIQUE, nom_scientifique TEXT, famille TEXT, origine TEXT,
        type_plante TEXT REFERENCES ref_type_plante(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        rusticite TEXT REFERENCES ref_rusticite(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        exposition_soleil TEXT REFERENCES ref_exposition_soleil(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        exposition_vent TEXT REFERENCES ref_exposition_vent(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        difficulte TEXT REFERENCES ref_difficulte(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        vitesse_croissance TEXT REFERENCES ref_vitesse_croissance(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        tolerance_secheresse TEXT REFERENCES ref_tolerance_secheresse(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        sensibilite_eau TEXT REFERENCES ref_sensibilite_eau(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        feuillage TEXT REFERENCES ref_feuillage(code) ON UPDATE CASCADE DEFAULT 'INCONNU',
        
        type_racinaire TEXT REFERENCES racines(type_racinaire) ON UPDATE CASCADE ON DELETE SET NULL,
        
        besoin_eau INTEGER, frequence_ete TEXT, frequence_hiver TEXT,
        floraison_debut INTEGER, floraison_fin INTEGER, recolte_debut INTEGER, recolte_fin INTEGER,
        toxicite_animaux BOOLEAN, toxicite_note TEXT, score_balcon INTEGER,
        volume_pot_min TEXT, dimensions_adulte TEXT, zone_climat TEXT,
        
        compagnonnage TEXT,
        conseil_arrosage TEXT, conseil_entretien TEXT, maladies TEXT, 
        vertus TEXT, precautions TEXT, notes TEXT, astuce_pro TEXT, multiplication TEXT
    );

    CREATE TABLE plante_alias (plante_id INTEGER REFERENCES plantes(id) ON DELETE CASCADE, nom_alternatif TEXT, PRIMARY KEY(plante_id, nom_alternatif));
    CREATE TABLE plante_sols (plante_id INTEGER REFERENCES plantes(id) ON DELETE CASCADE, type_sol TEXT REFERENCES sols(type_sol) ON UPDATE CASCADE, est_ideal BOOLEAN, PRIMARY KEY(plante_id, type_sol));
    CREATE TABLE plante_boutures (plante_id INTEGER REFERENCES plantes(id) ON DELETE CASCADE, type_bouture TEXT REFERENCES boutures(type_bouture) ON UPDATE CASCADE, PRIMARY KEY(plante_id, type_bouture));
    CREATE TABLE plante_compagnons (plante_id INTEGER REFERENCES plantes(id) ON DELETE CASCADE, compagnon_id INTEGER REFERENCES plantes(id) ON DELETE CASCADE, PRIMARY KEY(plante_id, compagnon_id));
    """
    con.executescript(schema_sql)

    # 2. INSERTION DES THEMES (Couleurs)
    print("🎨 Insertion des thèmes (Couleurs UI)...")
    theme_rows = []
    for theme_code, (r, g, b, a) in THEME.items():
        theme_rows.append((theme_code, r, g, b, a))
    con.executemany("INSERT OR IGNORE INTO ref_theme VALUES (?,?,?,?,?)", theme_rows)

    # 3. DICTIONNAIRES & ANTI-CRASH
    print("📖 Insertion des dictionnaires de référence...")
    REF_TABLE_MAP = {
        "NiveauDifficulte": "ref_difficulte",
        "TypePlante": "ref_type_plante",
        "ExpositionSoleil": "ref_exposition_soleil",
        "ExpositionVent": "ref_exposition_vent",
        "Rusticite": "ref_rusticite",
        "VitesseCroissance": "ref_vitesse_croissance",
        "ToleranceSecheresse": "ref_tolerance_secheresse",
        "SensibiliteEau": "ref_sensibilite_eau",
        "TypeFeuillage": "ref_feuillage",
    }

    for json_key, table_name in REF_TABLE_MAP.items():
        entries = ENUM_METADATA.get(json_key, [])
        # Filet de sécurité avec une couleur neutre (TextMuted)
        rows_to_insert = [('INCONNU', 'Inconnu', 'Information non renseignée', 'TextMuted', 99)]
        
        for (code, label, desc, theme_code, sort) in entries:
            # Sécurité si une couleur est mal tapée
            safe_theme_code = theme_code if theme_code in THEME else 'TextMuted'
            rows_to_insert.append((code, label, desc, safe_theme_code, sort))
            
        con.executemany(
            f"INSERT OR IGNORE INTO {table_name} "
            f"(code, label_ui, description, theme_code, sort_order) VALUES (?,?,?,?,?)",
            rows_to_insert
        )

    # GARANTIE "INCONNU" pour les tables liées
    con.execute("INSERT OR IGNORE INTO sols (type_sol) VALUES ('INCONNU')")
    con.execute("INSERT OR IGNORE INTO racines (type_racinaire) VALUES ('INCONNU')")
    con.execute("INSERT OR IGNORE INTO boutures (type_bouture) VALUES ('INCONNU')")

    # 4. INSERTION SOLS
    print("🌍 Création des Sols...")
    for s in load_json(json_dir / "sols.json"):
        ts = safe_str(s, "type_sol", "INCONNU").upper()
        ph = s.get("ph", {})
        con.execute("""INSERT OR REPLACE INTO sols (type_sol, texture, drainage, retention_eau, richesse, ph_min, ph_max, utilisation, cec, aeration, densite, tampon_ph, mineralisation, vie_microbienne, frequence_renouvellement, compatibilite_calcaire) 
                       VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)""", 
                    (ts, safe_str(s,"texture"), safe_str(s,"drainage"), safe_str(s,"retention_eau"), safe_str(s,"richesse"), 
                     ph.get("min",6.0) if type(ph)==dict else 6.0, ph.get("max",7.0) if type(ph)==dict else 7.0, safe_str(s,"utilisation"), safe_str(s,"cec"), safe_str(s,"aeration"), safe_str(s,"densite"), safe_str(s,"tampon_ph"), safe_str(s,"mineralisation"), safe_str(s,"vie_microbienne"), safe_str(s,"frequence_renouvellement"), safe_str(s,"compatibilite_calcaire")))
        for compo, pct in s.get("composition", {}).items():
            con.execute("INSERT OR IGNORE INTO sols_composition VALUES (?,?,?)", (ts, compo.upper(), float(pct)))
        for risque in s.get("risques", []):
            con.execute("INSERT OR IGNORE INTO sols_risques VALUES (?,?)", (ts, risque))

    # 5. INSERTION RACINES
    print("🫚 Création des Racines...")
    for r in load_json(json_dir / "racines.json"):
        tr = safe_str(r, "type_racinaire", "INCONNU").upper()
        vol = r.get("volume_litres", {})
        con.execute("""INSERT OR REPLACE INTO racines (type_racinaire, profondeur, largeur, volume_min, volume_max, forme_pot, drainage, frequence_rempotage, sensibilite_rempotage) 
                       VALUES (?,?,?,?,?,?,?,?,?)""",
                    (tr, safe_str(r,"profondeur_pot"), safe_str(r,"largeur_pot"), vol.get("min",0) if type(vol)==dict else 0, vol.get("max",0) if type(vol)==dict else 0, safe_str(r,"forme_pot"), safe_str(r,"drainage"), safe_str(r,"frequence_rempotage"), safe_str(r,"sensibilite_rempotage")))
        for mat in r.get("materiaux", []):
            con.execute("INSERT OR IGNORE INTO racines_materiaux VALUES (?,?)", (tr, mat))

    # 6. INSERTION BOUTURES
    print("✂️  Création des Boutures...")
    for b in load_json(json_dir / "boutures.json"):
        tb = safe_str(b, "nom", "INCONNU").upper()
        lg = b.get("longueur_coupe_cm") or {}
        tp = b.get("temperature_c") or {}
        en = b.get("temps_enracinement_jours") or {}
        con.execute("""INSERT OR REPLACE INTO boutures (type_bouture, difficulte, type_tige, longueur_min, longueur_max, substrat, hormone, humidite, temp_min, temp_max, enracinement_min, enracinement_max, taux_reussite) 
                       VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)""",
                    (tb, safe_str(b,"niveau_difficulte","INCONNU").upper(), safe_str(b,"type_tige"), lg.get("min") if type(lg)==dict else None, lg.get("max") if type(lg)==dict else None, safe_str(b,"substrat"), safe_str(b,"hormone_bouturage"), safe_str(b,"humidite"), tp.get("min",15) if type(tp)==dict else 15, tp.get("max",25) if type(tp)==dict else 25, en.get("min",0) if type(en)==dict else 0, en.get("max",0) if type(en)==dict else 0, safe_str(b,"taux_reussite")))
        for i, etape in enumerate(b.get("etapes", []), start=1):
            con.execute("INSERT OR IGNORE INTO boutures_etapes VALUES (?,?,?)", (tb, i, etape))
        for per in b.get("periode", []):
            con.execute("INSERT OR IGNORE INTO boutures_periodes VALUES (?,?)", (tb, per))

    # 7. INSERTION PLANTES
    print("🌿 Insertion des plantes...")
    for p in load_json(json_dir / "encyclopedia.json"):
        # Parsing robuste
        t_plante = safe_str(p, "type", "INCONNU").upper()
        rust = safe_str(p, "rusticite", "INCONNU").upper()
        expo = safe_str(p, "exposition_soleil", "INCONNU").upper()
        diff = safe_str(p, "niveau_difficulte", "INCONNU").upper()
        vit = safe_str(p, "vitesse_croissance", "INCONNU").upper()
        tol = safe_str(p, "tolerance_secheresse", "INCONNU").upper()
        sen = safe_str(p, "sensibilite_exces_eau", "INCONNU").upper()
        feu = safe_str(p, "feuillage", "INCONNU").upper()
        tr = safe_str(p, "type_racinaire_pot", "INCONNU").upper()

        # Anti-Crash
        con.execute("INSERT OR IGNORE INTO ref_type_plante (code) VALUES ('INCONNU')")
        con.execute("INSERT OR IGNORE INTO ref_type_plante (code) VALUES (?)", (t_plante,))
        con.execute("INSERT OR IGNORE INTO ref_rusticite (code) VALUES (?)", (rust,))
        con.execute("INSERT OR IGNORE INTO ref_exposition_soleil (code) VALUES (?)", (expo,))
        con.execute("INSERT OR IGNORE INTO ref_difficulte (code) VALUES (?)", (diff,))
        con.execute("INSERT OR IGNORE INTO ref_vitesse_croissance (code) VALUES (?)", (vit,))
        con.execute("INSERT OR IGNORE INTO ref_tolerance_secheresse (code) VALUES (?)", (tol,))
        con.execute("INSERT OR IGNORE INTO ref_sensibilite_eau (code) VALUES (?)", (sen,))
        con.execute("INSERT OR IGNORE INTO ref_feuillage (code) VALUES (?)", (feu,))
        con.execute("INSERT OR IGNORE INTO racines (type_racinaire) VALUES (?)", (tr,))

        cur = con.execute("""INSERT INTO plantes (
                nom, nom_scientifique, famille, origine, type_plante, rusticite, exposition_soleil, 
                exposition_vent, difficulte, vitesse_croissance, tolerance_secheresse, sensibilite_eau, feuillage, 
                type_racinaire, besoin_eau, frequence_ete, frequence_hiver, floraison_debut, floraison_fin, 
                recolte_debut, recolte_fin, toxicite_animaux, toxicite_note, score_balcon, volume_pot_min, 
                dimensions_adulte, zone_climat, compagnonnage, conseil_arrosage, conseil_entretien, maladies, vertus, precautions, notes, astuce_pro, multiplication
            ) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)""", (
            safe_str(p,"nom"), safe_str(p,"nom_scientifique"), safe_str(p,"famille"), safe_str(p,"origine"), t_plante, rust, expo, 
            safe_str(p,"exposition_vent").upper(), diff, vit, tol, sen, feu,
            tr, safe_int(p,"besoin_eau"), safe_str(p,"frequence_arrosage_ete"), safe_str(p,"frequence_arrosage_hiver"), safe_int(p,"floraison_debut"), safe_int(p,"floraison_fin"),
            safe_int(p,"recolte_debut"), safe_int(p,"recolte_fin"), safe_bool(p,"toxicite_animaux"), safe_str(p,"toxicite_note"), safe_int(p,"score_balcon"), safe_str(p,"volume_pot_min"),
            safe_str(p,"dimensions_adulte"), safe_str(p,"zone_climat"), safe_str(p,"compagnonnage"), safe_str(p,"conseil_arrosage"), safe_str(p,"conseil_entretien"), safe_str(p,"maladies"), safe_str(p,"vertus_medicinales"), safe_str(p,"precautions"), safe_str(p,"notes"), safe_str(p,"astuce_pro"), safe_str(p,"multiplication")
        ))
        pid = cur.lastrowid

        for alt in p.get("autres_noms", []):
            con.execute("INSERT OR IGNORE INTO plante_alias VALUES (?,?)", (pid, alt))
        
        for bk in p.get("boutures_compatibles", []):
            con.execute("INSERT OR IGNORE INTO boutures (type_bouture) VALUES (?)", (bk,))
            con.execute("INSERT OR IGNORE INTO plante_boutures VALUES (?,?)", (pid, bk))
            
        sol1 = safe_str(p, "sol_recommande", "INCONNU").upper()
        if sol1 and sol1 != "INCONNU":
            con.execute("INSERT OR IGNORE INTO sols (type_sol) VALUES (?)", (sol1,))
            con.execute("INSERT OR IGNORE INTO plante_sols VALUES (?,?,1)", (pid, sol1))

    # 8. LIAISONS COMPAGNONS
    print("🤝 Connexion des compagnonnages dynamiques...")
    con.executescript("""
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (1, 2);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (2, 1);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (4, 12);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (7, 11);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (11, 7);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (14, 1);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (14, 2);
        INSERT OR IGNORE INTO plante_compagnons (plante_id, compagnon_id) VALUES (15, 1);
    """)

    con.commit()
    con.close()
    print(f"\n✅ Migration parfaite terminée ! ({db_path.stat().st_size // 1024} Ko)")

if __name__ == "__main__":
    json_dir = pathlib.Path("Data/assets/JSON")
    db_path  = pathlib.Path("Data/assets/plantasia.db")
    if not json_dir.exists():
        print(f"❌ Dossier JSON introuvable : {json_dir.resolve()}", file=sys.stderr)
        sys.exit(1)
    migrate(json_dir, db_path)