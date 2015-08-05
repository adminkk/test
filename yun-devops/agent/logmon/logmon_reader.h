/**
 * copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 **/

/*
 * @file logmon_reader.h
 * @author zhaoxin08(zhaoxin08@baidu.com)
 * @date  2015-08-03
 * @brief
 *
 */

#ifndef LOGMON_LOGMON_READER_H
#define LOGMON_LOGMON_READER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string>

enum RET
{
    OK = 0,
    ERR = -1,
};

#define pinfo(fmt, ...)     \
    printf("\033[1;34;40m%s-%s-%d: "fmt"\033[0m\n", ((char*)__FILE__), \
            (char*)__func__, (int)__LINE__, ##__VA_ARGS__)

#define perr(fmt, ...)     \
    fprintf(stderr, "\033[1;31;40m%s-%s-%d: error: "fmt"\033[0m\n", \
            (char*)__FILE__, (char*)__func__, (int)__LINE__, ##__VA_ARGS__)

#define LM_NOTICE_LOG pinfo
#define LM_DEBUG_LOG pinfo
#define LM_WARNING_LOG perr

enum
{
    PATH_LENGTH = 1024
};

/**
 * @brief��־��ȡ��״̬�ļ�
 */
struct read_status_t
{
    off_t file_pos;   // !<��ȡ�ļ�λ��
    ino_t file_ino;   // !<��־�ļ���inode
};

/**
 * @brief �ж��ļ�����·���Ƿ����
 * @param [in] path
 * @return bool
 *        - true : ����
 *        - false��������
 */
bool is_path_exist(const char* path);

/**
 * @brief ģ����־��ȡ��
 *      ��ȡģ�����־���ڴ���
 */
class LmReader
{
public:
    /**
     * @brief ��ʼ����־��ȡ�����Ϣ
     * @param [in] status_file
     * @param [in] module_name
     * @param [in] log_path
     * @param [in] log_file
     * @param [in] new_log_file
     * @param [in] item_cycle
     *  ��������status�ļ����Ƶ�һ����
     */
    int init(const char* status_file,
             const char* module_name,
             const char* log_path,
             const char* log_file,
             const char* new_log_file,
             int item_cycle);
    /**
     * @brief ��ȡ��־�ļ�,�����ļ���С���ơ���ȡ�ٶȽ�������
     * @param [in] speed : int ��ȡ�ٶ�,��λ(bytes/s)
     * @param [in] dur_tm : int ���ζ�ȡ��ʱ��
     * @param [in/out] log_str : std::string ��ȡ����log�����ַ���
     * @return int
     *        - OK : �ɹ�
     *        - ERR�� ʧ��
     *
     **/
    int read_log(int speed, int dur_tm, std::string& log_str);
    /**
     * @brief��ȡ״̬�ļ�
     * @return int
     *        - OK : �ɹ�
     *        - ERR�� ʧ��
     */
    int read_status();
    /**
     * @brief��־��ȡ���������״̬�ļ�
     * @return int
     *        - OK : �ɹ�
     *        - ERR�� ʧ��
     */
    int update_status();
    /**
     * @brief ��ȡ��ʵ��
     */
    static LmReader* get_instance();
    /**
     * @brief �ж϶�ȡ����־�ļ��Ƿ񵽴��ļ�β
     * @return true :�����ļ�β
     *         false:û�е��ļ�β
     */
    bool is_eof();

    /**
     * @brief ����״̬�ļ��Ƿ�������жϳ����Ƿ��������
     * @return true : ��
     *         false: ��
     */
    bool is_first_launch();

    /**
     * @brief ����
     */
    void clear_up();

private:
    /**
     * @brief ���캯��
     */
    LmReader();
    ~LmReader();
    /**
     * @brief �ж���־�ļ��Ƿ��и�
     * @param st: struct stat* �ļ�stat
     * @return bool
     *        - true : �и�
     *        - false��  û�б��и�
     */
    bool is_split(struct stat* st);

    /**
     * @brief ���ո����Ķ�ȡ�ٶȶ�ȡ�ļ�
     * @param fp : FILE * ��ȡ�ļ�
     * @param speed : int ÿ���ȡ�ֽ���
     * @param dur_tm : int ��ÿ�����־�ĳ���ʱ��
     * @param str : std::string& ��ȡ���ַ���
     * @return int : ���ض�ȡ״̬
     */
    int speed_read(FILE * fp, int speed, int dur_tm, std::string& str);

    /**
     * @brief ͨ��inode�Ų�ȡ��Ӧ���ļ���
     *
     * @return
     *         OK : �ɹ�
     *         ERR: ʧ��
     */
    int find_last_log();

    /**
     * @brief ͨ��inode�������ұ��и�ǰ����־�ļ���
     *
     * @param fpath : ����Ŀ¼�е��ļ�����·��
     * @param sb : fpath��stat״̬
     * @param typeflag : ���ͱ�־
     *
     * @return
     */
    static int last_log_fun(const char *fpath,
                            const struct stat *sb,
                            int typeflag);
    ///������ҵĺ���ָ��
    typedef int (*last_log_fp)(const char *fpath,
                            const struct stat *sb,
                            int typeflag);
private:
    static read_status_t _s_read_status; //!< �ϸ�����log�ļ��Ķ�ȡ״̬
    FILE*  _status_file; //!< ��ȡ״̬�ļ�
    FILE*  _log_file;    //!< ��־�ļ�
    FILE*  _last_log_file;   //!< ���и���־����һ����־�ļ�
    FILE*  _cur_log_file;    //!< ��ǰ��ȡ��log�ļ�
    static LmReader * _s_lm_reader; //!<��ʵ������
    off_t  _cur_pos;    //!<��ǰ��־�ļ���ȡΪ��
    bool   _issplit;    //!<��־�ļ��Ƿ��и�
    size_t _log_size;   //!<����ȡ����־�ļ���С
    size_t _readlog_size; //!<��ǰ������Ҫ��ȡ��־�Ĵ�С
    char _log_path[PATH_LENGTH];    //!<��־�ļ�·��
    bool _st_file_exist;           //!<��־װ̬�ļ��Ƿ����
    static char _s_last_log_fn[PATH_LENGTH]; //!<���и���־�ļ���
    size_t _read_size;                //!< ��ȡ��־�ļ��Ĵ�С
    size_t _speed_read_size;          //!< speedread��ȡ����־�ļ���С
};

#endif

