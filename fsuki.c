/**nota: Si faltan metodos es porque la librería estandar de itsuki ya los cubre
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#ifdef _WIN32
  #include	<direct.h>
  #include	<windows.h>
  #define FS_MKDIR(p)     _mkdir(p)
  #define FS_RMDIR(p)     _rmdir(p)
  #define FS_CHDIR(p)     _chdir(p)
  #define FS_GETCWD(b, n) _getcwd(b, n)
#else
  #include	<unistd.h>
  #include	<dirent.h>
  #define FS_MKDIR(p)     mkdir(p, 0755)
  #define FS_RMDIR(p)     rmdir(p)
  #define FS_CHDIR(p)     chdir(p)
  #define FS_GETCWD(b, n) getcwd(b, n)
#endif

/** 1. Metadata */

int
fs_exists(const char *path)
{
	struct stat st;
	if(path == NULL)
		return 0;
	return (stat(path, &st) == 0) ? 1 : 0;
}

int
fs_is_file(const char *path)
{
	struct stat st;
	if(path == NULL)
		return 0;
	return (stat(path, &st) == 0 && S_ISREG(st.st_mode)) ? 1 : 0;
}

int
fs_size(const char *path)
{
	// https://pubs.opengroup.org/onlinepubs/9699919799/
	struct stat st;
	if(path == NULL)
		return -1;
	if(stat(path, &st) != 0)
		return -1;
	return (int)st.st_size;
}

/** 2. Directories */

int
fs_mkdir(const char *path)
{
	if(path == NULL)
		return -1;
	if(FS_MKDIR(path) != 0)
		return -1;
	return 0;
}

int
fs_mkdirs(const char *path)
{
	if(path == NULL)
		return -1;

	char tmp[1024];
	int len;
	int i;

	len = (int)strlen(path);
	if(len <= 0 || len >= (int)sizeof(tmp))
		return -1;

	strcpy(tmp, path);
	for(i = 1; i < len; i++){
		if(tmp[i] == '/'){
			tmp[i] = '\0';
			if(FS_MKDIR(tmp) != 0 && errno != EEXIST)
				return -1;
			tmp[i] = '/';
		}
	}
	if(FS_MKDIR(tmp) != 0 && errno != EEXIST)
		return -1;

	return 0;
}

int
fs_rmdir(const char *path)
{
	if(path == NULL)
		return -1;
	if(FS_RMDIR(path) != 0)
		return -1;
	return 0;
}

int
fs_remove_all(const char *path)
{
	struct stat st;

	if(path == NULL || path[0] == '\0')
		return -1;

	if(stat(path, &st) != 0)
		return -1;

	if(!(st.st_mode & S_IFDIR)){
#ifdef _WIN32
		SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
#endif
		return (remove(path) == 0) ? 0 : -1;
	}

#ifdef _WIN32
	{
		WIN32_FIND_DATAA fd;
		HANDLE h;
		char pattern[1024];
		char subpath[1024];
		int plen = (int)strlen(path);

		if(plen + 4 >= (int)sizeof(pattern))
			return -1;

		sprintf(pattern, "%s/*", path);
		h = FindFirstFileA(pattern, &fd);
		if(h != INVALID_HANDLE_VALUE){
			do{
				const char *name = fd.cFileName;
				if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
					continue;
				if(plen + 1 + (int)strlen(name) >= (int)sizeof(subpath)){
					FindClose(h);
					return -1;
				}
				sprintf(subpath, "%s/%s", path, name);
				if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					if(fs_remove_all(subpath) != 0){
						FindClose(h);
						return -1;
					}
				} else {
					if(fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
						SetFileAttributesA(subpath, FILE_ATTRIBUTE_NORMAL);
					if(remove(subpath) != 0){
						FindClose(h);
						return -1;
					}
				}
			}while(FindNextFileA(h, &fd));
			FindClose(h);
		}
		SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
		return (FS_RMDIR(path) == 0) ? 0 : -1;
	}
#else
	{
		DIR *d = opendir(path);
		struct dirent *e;
		char subpath[1024];
		int plen = (int)strlen(path);

		if(d == NULL)
			return -1;

		while((e = readdir(d)) != NULL){
			if(strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
				continue;
			if(plen + 1 + (int)strlen(e->d_name) >= (int)sizeof(subpath)){
				closedir(d);
				return -1;
			}
			sprintf(subpath, "%s/%s", path, e->d_name);
			if(fs_remove_all(subpath) != 0){
				closedir(d);
				return -1;
			}
		}
		closedir(d);
		return (FS_RMDIR(path) == 0) ? 0 : -1;
	}
#endif
}

int
fs_rmdir_all(const char *path)
{
	return fs_remove_all(path);
}

int
fs_list(const char *path, char *out, int out_len)
{
#ifdef _WIN32
	// https://docs.originlab.com/originc/ref/win32_find_dataa/
	WIN32_FIND_DATAA fd;
	HANDLE h;
	char pattern[1024];
	int used = 0, count = 0, plen;
#else
	DIR *d;
	struct dirent *e;
	int used = 0, count = 0;
#endif
	if(path == NULL || out == NULL || out_len < 2)
		return -2;
#ifdef _WIN32
	plen = (int)strlen(path);
	if(plen + 4 >= (int)sizeof(pattern))
		return -2;
	sprintf(pattern, "%s/*", path);
	h = FindFirstFileA(pattern, &fd);
	if(h == INVALID_HANDLE_VALUE)
		return -1;
	do{
		const char *name = fd.cFileName;
		int nlen;
		if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;
		nlen = (int)strlen(name);
		if(used + nlen + 2 > out_len){
			FindClose(h);
			return -2;
		}
		memcpy(out + used, name, nlen);
		used += nlen;
		out[used++] = '\n';
		count++;
	}while(FindNextFileA(h, &fd));
	FindClose(h);
#else
	d = opendir(path);
	if(d == NULL)
		return -1;
	while((e = readdir(d)) != NULL){
		int nlen;
		if(strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
			continue;
		nlen = (int)strlen(e->d_name);
		if(used + nlen + 2 > out_len){
			closedir(d);
			return -2;
		}
		memcpy(out + used, e->d_name, nlen);
		used += nlen;
		out[used++] = '\n';
		count++;
	}
	closedir(d);
#endif

	out[used] = '\0';
	return count;
}

int
fs_count(const char *path)
{
	char tmp[8192];
	if(path == NULL)
		return -1;
	return fs_list(path, tmp, (int)sizeof(tmp));
}

/** 3. Files */

int
fs_copy(const char *src, const char *dst)
{
	FILE *in, *out;
	char buf[8192];
	int total = 0, n;

	if(src == NULL || dst == NULL)
		return -1;
	if(strcmp(src, dst) == 0)
		return -1;

	in = fopen(src, "rb");
	if(in == NULL)
		return -2;
	out = fopen(dst, "wb");
	if(out == NULL){
		fclose(in);
		return -1;
	}

	while((n = (int)fread(buf, 1, sizeof(buf), in)) > 0){
		if(fwrite(buf, 1, (size_t)n, out) != (size_t)n){
			fclose(in);
			fclose(out);
			return -1;
		}
		total += n;
	}
	fclose(in);
	fclose(out);
	return total;
}

int
fs_touch(const char *path)
{
	FILE *f;
	if(path == NULL)
		return -1;
	f = fopen(path, "ab");
	if(f == NULL)
		return -1;
	fclose(f);
	return 0;
}

int
fs_remove(const char *path)
{
	if(path == NULL)
		return -1;
	if(remove(path) != 0)
		return -1;
	return 0;
}

int
fs_rename(const char *old_path, const char *new_path)
{
	if(old_path == NULL || new_path == NULL)
		return -1;
	if(rename(old_path, new_path) != 0)
		return -1;
	return 0;
}

/** 4. env */

int
fs_cwd(char *buf, int buf_len)
{
	if(buf == NULL || buf_len <= 0)
		return -1;
	if(FS_GETCWD(buf, buf_len) == NULL)
		return (errno == ERANGE) ? -2 : -1;
	return 0;
}

int
fs_chdir(const char *path)
{
	if(path == NULL)
		return -1;
	if(FS_CHDIR(path) != 0)
		return -1;
	return 0;
}

/** 5. routes */

int
fs_join(const char *a, const char *b, char *out, int out_len)
{
	int alen, blen, need_sep, used = 0;

	if(a == NULL || b == NULL || out == NULL || out_len < 2)
		return -1;
	alen = (int)strlen(a);
	blen = (int)strlen(b);
	need_sep = (alen > 0 && blen > 0 && a[alen-1] != '/' && b[0] != '/') ? 1 : 0;
	if(alen + need_sep + blen + 1 > out_len)
		return -2;
	memcpy(out, a, alen);
	used = alen;
	if(need_sep)
		out[used++] = '/';
	memcpy(out + used, b, blen);
	out[used + blen] = '\0';
	return 0;
}

int
fs_basename(const char *path, char *out, int out_len)
{
	const char *base;
	int len;

	if(path == NULL || out == NULL)
		return -1;
	base = strrchr(path, '/');
#ifdef _WIN32
	{
		const char *bs = strrchr(path, '\\');
		if(bs != NULL && (base == NULL || bs > base))
			base = bs;
	}
#endif
	base = (base != NULL) ? base + 1 : path;
	len = (int)strlen(base);
	if(len + 1 > out_len)
		return -2;
	strcpy(out, base);
	return 0;
}

int
fs_dirname(const char *path, char *out, int out_len)
{
	const char *last;
	int len;

	if(path == NULL || out == NULL)
		return -1;
	last = strrchr(path, '/');
#ifdef _WIN32
	{
		const char *bs = strrchr(path, '\\');
		if(bs != NULL && (last == NULL || bs > last))
			last = bs;
	}
#endif
	if(last == NULL){
		if(out_len < 2)
			return -2;
		strcpy(out, ".");
		return 0;
	}
	len = (int)(last - path);
	if(len == 0){
		if(out_len < 2)
			return -2;
		strcpy(out, "/");
		return 0;
	}
	if(len + 1 > out_len)
		return -2;
	memcpy(out, path, len);
	out[len] = '\0';
	return 0;
}

int
fs_ext(const char *path, char *out, int out_len)
{
	char name[1024];
	const char *dot;
	int len, r = fs_basename(path, name, (int)sizeof(name));

	if(r != 0)
		return r;
	dot = strrchr(name, '.');
	if(dot == NULL || dot == name){
		if(out_len < 1)
			return -2;
		out[0] = '\0';
		return 0;
	}
	len = (int)strlen(dot + 1);
	if(len + 1 > out_len)
		return -2;
	strcpy(out, dot + 1);
	return 1;
}
