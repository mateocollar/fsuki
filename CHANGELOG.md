# Changelog

## [1.0.0] - 2026-10-23
### Añadido
- **Compatibilidad multiplataforma**: mismo código fuente compila en Windows y Linux, sin librerías externas.
- **Metadatos**: `fs_exists()`, `fs_is_file()`, `fs_size()`.
- **Directorios**: `fs_mkdir()`, `fs_mkdirs()` (creación anidada estilo `mkdir -p`), `fs_rmdir()`, `fs_list()` (nombres separados por `\n`, sin `.` ni `..`) y `fs_count()`.
- **Archivos**: `fs_copy()` (copia binaria con retorno de bytes), `fs_touch()`, `fs_remove()`, `fs_rename()`.
- **Entorno**: `fs_cwd()` y `fs_chdir()`.
- **Utilidades de rutas**: `fs_join()`, `fs_basename()`, `fs_dirname()`, `fs_ext()` — todas con separador `/` (aceptado por Windows).
- **Documentación completa** de cada función en `fsuki.huki`, con códigos de error por función.
- Test autocontenido (`test_fsuki.suki`) que crea, lista, copia y limpia una estructura de prueba.

### Decisiones de diseño
- No duplica la lectura/escritura de contenido: `file_read_all`/`file_write_all`/`file_append` de `fundamental.huki` siguen siendo la vía para contenido.
- No requiere `init`/`cleanup` (a diferencia de sukets, no hay estado global de SO que inicializar).
- Convención de rutas con separador `/` en ambas plataformas.

### Limitaciones conocidas
- El codegen de Itsuki 5.5.0 allocatea `char[N]` como `4·N` bytes de stack y no emite *stack probing*: frames de más de 4 KB pueden segfaultear en Windows. Los ejemplos y el test de fsuki se mantienen por debajo de ese límite mientras se corrige upstream.
