/*
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 *
 */

/*
 * @file log_reader.cc
 * @author zhaoxin08(zhaoxin08@baidu.com)
 * @date  2015-8-3
 * @brief
 *      ��ȡ��־�ļ�,���Դ���ָ����־,������־��ȡ�ٶȽ��п���
 */

#include <errno.h>
#include <ftw.h>
#include <sys/time.h>
#include "logmon_reader.h"

LmReader* LmReader::_s_lm_reader = NULL;
read_status_t LmReader::_s_read_status = {0, 0};
char LmReader::_s_last_log_fn[] = {0};

static void string_replace(char* str, char old_char, char new_char)
{
    if (NULL == str || 0 == strlen(str)) {
        return;
    }

    while (str = strchr(str, old_char)) {
        *str = new_char;
    }
}

bool is_path_exist(const char* path)
{
    bool ret = false;
    if (NULL == path || '\0' == *path){
        return ret;
    }

    ///ͨ��װ̬�ж�path�Ƿ����
    struct stat statbuf;
    if (stat(path, &statbuf) == 0) {
        ret = true;
    } else if (errno != ENOENT) {
        LM_NOTICE_LOG("is_path_exist error, [path:%s]\n", path);
    }

    return ret;
}

int make_rec_dir(const char* path)
{
    int ret = 0;
    if (NULL == path || '\0' == *path) {
        return -1;
    }
    if (is_path_exist(path)) {
        return 0;
    }

    char cmd[PATH_LENGTH] = {0};
    snprintf(cmd, PATH_LENGTH, "mkdir -p %s", path);
    if (-1 == (ret = system(cmd))) {
        LM_WARNING_LOG("make rec dir error");
        ret = -1;
    }

    return ret;
}

LmReader::LmReader() :
    _status_file(NULL),
    _log_file(NULL),
    _last_log_file(NULL),
    _cur_log_file(NULL),
    _cur_pos(0),
    _issplit(false),
    _log_size(0),
    _readlog_size(0),
    _st_file_exist(true),
    _read_size(0)
{
    memset(_log_path, 0, PATH_LENGTH);
}

LmReader::~LmReader() {}

int LmReader::init(const char* status_file,
                   const char* module_file,
                   const char* log_path,
                   const char* log_file,
                   const char* new_log_path,
                   int ite_cycle)
{
    if (NULL == status_file ||
        NULL == module_file ||
        NULL == log_file ||
        NULL == new_log_path)
    {
        LM_WARNING_LOG("wrong log_file param");
        return ERR;
    }
    //����module_name��log_path,log_file����״̬�ļ���
    //NOTE: ��ͬ��ʵ����ȡ��״̬�ļ�Ӧ�ô���ڲ�ͬ���ļ���
    char st_file_name[PATH_LENGTH] = {0};
    char log_path_str[PATH_LENGTH] = {0};
    snprintf(log_path_str, PATH_LENGTH, "%s", log_path);
    string_replace(log_path_str, '/', '_');
    snprintf(st_file_name, PATH_LENGTH, "%s_%s_%s_%s.dat", status_file,
             module_file, log_path_str, log_file);
    LM_NOTICE_LOG("read status file name: %s", st_file_name);
    //NOTE: Ϊ��fix���ϵĲ�ͬ������һ��״̬�ļ�bug,���Ӳ���������Ϊkey����״̬�ļ���
    char new_st_file_name[PATH_LENGTH] = { 0 };
    snprintf(log_path_str, PATH_LENGTH, "%s", new_log_path);
    string_replace(log_path_str, '/', '_');
    snprintf(new_st_file_name, PATH_LENGTH, "%s_%s_%s_%d.dat", status_file,
             module_file, log_path_str, ite_cycle);
    LM_NOTICE_LOG("new read status file name: %s", new_st_file_name);
    //��״̬�ļ�����ȡ״̬
    if (!is_path_exist(st_file_name) &&
        !is_path_exist(new_st_file_name))
    {
        _status_file = fopen(new_st_file_name, "w+");
        _st_file_exist = false;
    } else {
        rename(st_file_name, new_st_file_name);
        _status_file = fopen(new_st_file_name, "r+");
    }
    if (!_status_file) {
        LM_WARNING_LOG("open status file failed");
        fprintf(stdout, "logmon_error:open_status_file_error\n");
        return ERR;
    }

    if (OK != read_status()) {
        LM_WARNING_LOG("read status file failed");
        fprintf(stdout, "logmon_error:read_status_file_error\n");
        return ERR;
    }

    //�趨��ǰ��־��ȡλ��Ϊ�ϴζ�ȡλ��
    _cur_pos = _s_read_status.file_pos;
    LM_NOTICE_LOG("[read_status][file pos:%ld][file inode:%ld][cur pos:%ld]",
                  _s_read_status.file_pos, _s_read_status.file_ino, _cur_pos);

    //��־�ļ�·����ʼ��
    snprintf(_log_path, PATH_LENGTH, "%s", log_path);

    //����־�ļ�
    char log_file_name[PATH_LENGTH] = { 0 };
    snprintf(log_file_name, PATH_LENGTH, "%s/%s", log_path, log_file);
    LM_NOTICE_LOG("log file name:%s", log_file_name);
    if (!is_path_exist(log_file_name)) {
        LM_WARNING_LOG("log_file could not be accessed");
        fprintf(stdout, "logmon_error:access_log_error\n");
        return ERR;
    }
    _log_file = fopen(log_file_name, "r");
    if (_log_file) {
        LM_WARNING_LOG("open log file failed:%s", strerror(errno));
        return ERR;
    }
    //��ȡ��־�ļ���С
    struct stat st;
    if (-1 == fstat(fileno(_log_file), &st)) {
        LM_WARNING_LOG("get file stat error:%s", strerror(errno));
        return ERR;
    }
    _log_size = st.st_size;
    LM_NOTICE_LOG("log size:%ld", _log_size);
    //���õ�ǰ��ȡ��־
    _cur_log_file = _log_file;

    long long last_log_size = 0;
    //�����־���ָ�Ļ�,����ұ��ָ����־��ȡ
    if (is_split(&st)) {
        _issplit = true;
        LM_NOTICE_LOG("log file is splitted");
        //���ҷָ�ǰ���ļ�,��ȡ�ָ�ʱδ��ȡ���ļ�
        if (OK == find_last_log()) {
            LM_NOTICE_LOG("log file is splitted,last log file name is %s", _s_last_log_fn);
            _last_log_file = fopen(_s_last_log_fn, "r");
            if (NULL == _last_log_file) {
                LM_WARNING_LOG("open last log failed, use current log");
                _cur_pos = 0;
            } else if (fileno(_last_log_file) != fileno(_cur_log_file)) {
                _cur_log_file = _last_log_file;
                struct stat st;
                if (-1 == fstat(fileno(_last_log_file), &st)) {
                    LM_WARNING_LOG("get file stat error:%s", strerror(errno));
                }
                last_log_size = st.st_size;
            }
        }
    }
    // ���㵱ǰ�ɼ�������Ҫ��ȡ����־��С
    _readlog_size = _log_size;
    if (_issplit) {
        // ����������ļ��ָ��size��Ҫ�����ָ��ȥ���ļ���δ�����ֵĴ�С
        if (last_log_size > _s_read_status.file_pos) {
            _readlog_size += last_log_size - _s_read_status.file_pos;
            LM_NOTICE_LOG("split logsize: %zu :last_log_size: %lld"
                            " read_status: %zu readlog_size:%zu",
                            _log_size, last_log_size, _s_read_status.file_pos, _readlog_size);
        }
    } else {
        // ��ȥ�Ѷ����ֵ�size
        _readlog_size -= _s_read_status.file_pos;
        LM_NOTICE_LOG("logsize: %ld :last_log_size: %lld read_status: %ld readlog_size:%ld",
                      _log_size, last_log_size, _s_read_status.file_pos, _readlog_size);
    }

    return OK;
}

int LmReader::read_status()
{
    //NOTE����ȡ״̬�ļ������ڣ���Ϊ�����һ�ζ�ȡ��־
    //      Ϊ�˴���򵥣�ֱ��������־��ȡ�����Ĭ��ֵ
    //      ����ļ���СΪ0��ҲĬ�ϵ�ǰ��ȡ��־λ��Ϊ0
    _s_read_status.file_pos = 0;
    _s_read_status.file_ino = 0;
    struct stat st;
    if (-1 == fstat(fileno(_status_file), &st)) {
        LM_WARNING_LOG("get file stat error:%s", strerror(errno));
        return ERR;
    }
    if (0 == st.st_size || !_st_file_exist) {
        return OK;
    }
    if (0 != fseek(_status_file, 0, SEEK_SET)) {
        LM_WARNING_LOG("[read_status]seek to the begin of status file error:%s",
                       strerror(errno));
        return ERR;
    }

    int ret = 0;
    ret = fscanf(_status_file, "file_pos:%ld\n", &_s_read_status.file_pos);
    if (1 != ret) {
        LM_WARNING_LOG("[read_status]file_pos,error:%s", strerror(errno));
        return ERR;
    }
    ret = fscanf(_status_file, "file_ino:%ld\n", &_s_read_status.file_ino);
    if (1 != ret) {
        LM_WARNING_LOG("[read_status]error:%s", strerror(errno));
        return ERR;
    }

    return OK;
}

bool LmReader::is_split(struct stat* st)
{
    //�ж��ļ��Ƿ�ָ�,���״̬�ļ�������,����Ϊ��1������,û�б��и�
    if (_st_file_exist
        &&  (st->st_size < _s_read_status.file_pos
        || st->st_ino != _s_read_status.file_ino)) {
        return true;
    }

    return false;
}

int LmReader::update_status()
{
    struct stat st;
    if (-1 == fstat(fileno(_log_file), &st)) {
        LM_WARNING_LOG("[update_status]get file stat error:%s", strerror(errno));
        return ERR;
    }
    _s_read_status.file_ino = st.st_ino;
    _s_read_status.file_pos = _log_size;
    rewind(_status_file);
    //д״̬֮ǰ���״̬�ļ��о�����,���ļ�truncate��0
    if (0 != ftruncate(fileno(_status_file), 0)) {
        LM_WARNING_LOG("truncate status file error");
    }
    //�����Ƶ��ļ�ͷ��д��״̬
    rewind(_status_file);
    fprintf(_status_file, "file_pos:%ld\n", _s_read_status.file_pos);
    fprintf(_status_file, "file_ino:%ld\n", _s_read_status.file_ino);
    fflush(_status_file);
    LM_NOTICE_LOG("[update_status][file pos:%ld][file inode:%ld]",
                  _s_read_status.file_pos, _s_read_status.file_ino);
    return OK;
}

int LmReader::read_log(int speed, int dur_tm, std::string& log_str)
{
    //NOTE:������и�Ҷ�ȡ���ļ�����,�õ�ǰ��ȡlog�ļ�Ϊ��ǰ��־
    if (_issplit) {
        if (NULL != _last_log_file && _log_file != _cur_log_file) {
            if (feof(_last_log_file)) {
                _cur_log_file = _log_file;
                _cur_pos = 0;
                LM_NOTICE_LOG("[read_log]read split log finish");
            }
        }
    }
    LM_DEBUG_LOG("[read_log][speed:%d][read duration:%d][cur file:%p]",
                 speed, dur_tm, _cur_log_file);
    if (!_st_file_exist) {
        _cur_pos = _log_size;
    }
    //��ȡʣ����־
    fseek(_cur_log_file, _cur_pos, SEEK_SET);
    if (feof(_cur_log_file)) {
        LM_DEBUG_LOG("should arrive the eof in the first time run");
    }
    if (OK != speed_read(_cur_log_file, speed, dur_tm, log_str)) {
        LM_WARNING_LOG("read log file error");
    }
    //�趨��ǰ�ļ���ȡָ��,��Ҫ�ƶ�������
    size_t last_line = log_str.rfind('\n');
    size_t log_str_size = log_str.size();
    size_t back_move_size = 0;
    if (std::string::npos != last_line) {
        back_move_size =  log_str_size - (last_line + 1);
    } else {
        back_move_size = 0;
    }
    _cur_pos = ftell(_cur_log_file) - back_move_size;
    _read_size +=  _speed_read_size - back_move_size;
    if (back_move_size != 0) {
        log_str = log_str.substr(0, last_line+1);
    }
    LM_DEBUG_LOG("[read_log][size:%zu][act size:%zu][back size:%zu][str is:",
                 _speed_read_size, _read_size, back_move_size);
    LM_DEBUG_LOG("%s", log_str.c_str());

    return OK;
}

LmReader *LmReader::get_instance()
{
    if (NULL == _s_lm_reader) {
        _s_lm_reader = new LmReader();
    }

    return _s_lm_reader;
}

bool LmReader::is_eof()
{
    //����ȡ���ļ���С�����˲ɼ��Ĵ�С��ʱ�����Ϊ��ȡ������
    //���ⲻ����feof���жϣ�feof�Ὣ�����ɲ���Ҳ���ȥ
    return (_read_size >= _readlog_size) ? true : false;
}

bool LmReader::is_first_launch()
{
    return !_st_file_exist;
}

void LmReader::clear_up()
{
    if (NULL != _status_file) {
        fclose(_status_file);
    }
    if (NULL != _log_file) {
        fclose(_log_file);
    }
    if (NULL != _last_log_file) {
        fclose(_last_log_file);
    }
    if (NULL != _s_lm_reader) {
        delete _s_lm_reader;
    }
    _s_lm_reader = NULL;
}

int LmReader::speed_read(FILE* fp, int speed, int dur_tm, std::string& str)
{
    if (dur_tm > 500) {
        LM_WARNING_LOG("read duration is too long, %d(ms)", dur_tm);
    }
    struct timeval start_tm;
    struct timeval end_tm;
    struct timeval used_tm;
    struct timespec sleep_tm = {0, 0};
    const int READ_COUNT = 1; // READ_COUNT �������ֵ˵�����߻����������ζ�ȡ
    //ת����ÿ�ζ�ȡ�ֽ���: speed(byte/s) * dur_tm(ms) / (1000 * READ_COUNT)
    int read_byte = speed * dur_tm / (1000 * READ_COUNT);
    int i = 0;
    int nr = 0;
    _speed_read_size = 0;
    //2097=(1024*1024byte/50)*100(ms)/(1000 * 1),Ĭ��Ϊ1M��50���������
    read_byte = read_byte <= 0 ? 2097 : read_byte;
    char* buf = new char[read_byte * READ_COUNT + 1];
    LM_DEBUG_LOG("[speed read][read byte:%d]", read_byte);
    size_t last_line = str.rfind('\n');
    for (i = 0; i < READ_COUNT; i++) {
        gettimeofday(&start_tm, NULL);
        clearerr(fp);
        nr = fread(buf +  _speed_read_size, 1, (int)read_byte, fp);
        if (ferror(fp)) {
            LM_WARNING_LOG("read file error:%s", strerror(errno));
            delete [] buf;
            return ERR;
        }
        _speed_read_size += nr;
        if (nr < read_byte) { //�����ļ���β
            break;
        }
        gettimeofday(&end_tm, NULL);
        timersub(&end_tm, &start_tm, &used_tm);
        //FIXME��it's ugly but useful ,it should be fixed in the future
        sleep_tm.tv_nsec = 1000000 * ((dur_tm - 20) / READ_COUNT
                           - (used_tm.tv_sec * 1000 + used_tm.tv_usec / 1000));
        if (sleep_tm.tv_nsec < 0) {
            sleep_tm.tv_nsec = 0;
        }
        nanosleep(&sleep_tm, NULL);
    }
    LM_DEBUG_LOG("[speed read][need to read:%d][actually read size:%ld]",
                 read_byte * READ_COUNT, _speed_read_size);
    buf[_speed_read_size] = '\0';
    if (std::string::npos != last_line) {
        // str.setf("%s", buf);
        str.assign(buf);
    } else {
        str.append(buf);
    }
    delete [] buf;

    return OK;
}

int LmReader::find_last_log()
{
    if (-1 == ftw(_log_path, last_log_fun, 100)) {
        LM_WARNING_LOG("find last_log_file from inode failed");
        return ERR;
    }

    return OK;
}

int LmReader::last_log_fun(const char *fpath,
                            const struct stat *sb,
                            int typeflag)
{
    if (sb->st_ino == _s_read_status.file_ino) {
        // size_t size = strlen(fpath);
        // size = size < PATH_LENGTH ? size : PATH_LENGTH;
        // strncpy(_s_last_log_fn, fpath, size);
        snprintf(_s_last_log_fn, PATH_LENGTH, "%s", fpath);
        // _s_last_log_fn[size - 1] = '\0';

        LM_NOTICE_LOG("find last_log_file from inode success.name:%s",
                      _s_last_log_fn);
        return 1;
    }

    return 0;
}

