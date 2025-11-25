---
trigger: always_on
---

1. On utilise CMake
2. Les fichiers sources sont tous dans src/
3. Pour créer un nouveau code qui fait des actions qui n'ont rien avoir avec le reste, on crée une classe
4. Le code doit être commenté seulement quant il n'est pas intuitif
5. CMakeLists.txt se trouve dans le dossier racine du projet, n'oublie pas de l'actualiser si besoin
6. Teste tes modifications une fois que tu as fini de les générer : utilise 'make' dans le dossier build/ puis "./3D-Sandbox"
7. Utilise des logs pour les fonctions/méthodes