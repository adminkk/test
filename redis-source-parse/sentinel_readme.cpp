
0 ��Ҫ���ݽṹ

struct sentinelState {
    uint64_t current_epoch;   // ��ǰ���ڵڼ������ͣ�ÿ��fail over��current_epoch+1��
    dict *masters;            // masterʵ���ֵ䣨һ��sentinle�ɼ�ض��master����key��sri��name��value��sri
    int tilt;                 // �Ƿ���TITLģʽ�У�������ϸ����TITLģʽ
    int running_scripts;      // ��ǰ����ִ�еĽű�
    mstime_t tilt_start_time; // TITLģʽ��ʼ��ʱ�� */
    mstime_t previous_time;   // �ϴ�ִ��sentinel������ִ�������ʱ�䣬�����ж��Ƿ����TITLģʽ
    list *scripts_queue;      // ��ִ�нű�����
    char *announce_ip;        // ִ��gossipЭ���ʱ���ʶsentinel��ip
    int announce_port;        // ִ��gossipЭ���ʱ���ʶsentinel��port
} sentinel;

typedef struct sentinelRedisInstance {
    int flags;                 // sentinel����ݣ��鿴SRI_... ϵ�еĶ���
    char *name;                // sri������
    char *runid;               // sri��ʵ��
    uint64_t config_epoch;     // Configuration epoch.
    sentinelAddr *addr;        // Master host.
    redisAsyncContext *cc;     // Hiredis context for commands
    redisAsyncContext *pc;     // Hiredis context for Pub / Sub
    int pending_commands;      // �ȴ��ظ�������ĸ���
    mstime_t cc_conn_time;     // cc connection time.
    mstime_t pc_conn_time;     // pc connection time.
    mstime_t pc_last_activity; // Last time we received any message.
    mstime_t last_avail_time;  // Last time the instance replied to ping with a reply we consider valid.
    mstime_t last_ping_time;  /* Last time a pending ping was sent in the
                              context of the current command connection
                              with the instance. 0 if still not sent or
                              if pong already received. */
    mstime_t last_pong_time;  /* Last time the instance replied to ping,
                              whatever the reply was. That's used to check
                              if the link is idle and must be reconnected. */
    mstime_t last_pub_time;   /* Last time we sent hello via Pub/Sub. */
    mstime_t last_hello_time; /* Only used if SRI_SENTINEL is set. Last time
                              we received a hello from this Sentinel
                              via Pub/Sub. */
    mstime_t last_master_down_reply_time; /* Time of last reply to
                                          SENTINEL is-master-down command. */
    mstime_t s_down_since_time; /* Subjectively down since time. */
    mstime_t o_down_since_time; /* Objectively down since time. */
    mstime_t down_after_period; /* Consider it down after that period. */
    mstime_t info_refresh;  /* Time at which we received INFO output from it. */

    /* Role and the first time we observed it.
    * This is useful in order to delay replacing what the instance reports
    * with our own configuration. We need to always wait some time in order
    * to give a chance to the leader to report the new configuration before
    * we do silly things. */
    int role_reported;
    mstime_t role_reported_time;
    mstime_t slave_conf_change_time; /* Last time slave master addr changed. */

    /* Master specific. */
    dict *sentinels;                 // ���ͬһ��ri master������sentinelʵ���Ĺ�ϣ��
    dict *slaves;                    // ri master ��slave ��ϣ��
    unsigned int quorum;             // ͬ��ri master��sdown����odown״̬������Ʊ��
    int parallel_syncs;              // ͬʱ���Խ��ж��ٸ�failover
    char *auth_pass;                 // auth��password

    /* Slave specific. */
    mstime_t master_link_down_time;  // Slave replication link down time. */
    int slave_priority; /* Slave priority according to its INFO output. */
    mstime_t slave_reconf_sent_time; /* Time at which we sent SLAVE OF <new> */
    struct sentinelRedisInstance *master; // slave sri��master sri
    char *slave_master_host;              // slave ri��host[��info�����ȡ��]
    int slave_master_port;                // slave ri��port[��info�����ȡ��]
    int slave_master_link_status;         // slave ri������״̬
    unsigned long long slave_repl_offset; // slave ri��replication offset

    /* Failover */
    char *leader;       /* If this is a master instance, this is the runid of
                        the Sentinel that should perform the failover. If
                        this is a Sentinel, this is the runid of the Sentinel
                        that this Sentinel voted as leader. */
    uint64_t leader_epoch; /* Epoch of the 'leader' field. */
    uint64_t failover_epoch; /* Epoch of the currently started failover. */
    int failover_state; /* See SENTINEL_FAILOVER_STATE_* defines. */
    mstime_t failover_state_change_time;
    mstime_t failover_start_time;   /* Last failover attempt start time. */
    mstime_t failover_timeout;      /* Max time to refresh failover state. */
    mstime_t failover_delay_logged; /* For what failover_start_time value we
                                    logged the failover delay. */
    struct sentinelRedisInstance *promoted_slave; /* Promoted slave instance. */
    /* Scripts executed to notify admin or reconfigure clients: when they
    * are set to NULL no script is executed. */
    char *notification_script;
    char *client_reconfig_script;
} sentinelRedisInstance;

1 main

int main(int argc, char **argv) {     
    // sentinelģʽ�ж�  
    server.sentinel_mode = checkForSentinelMode(argc,argv);    
  
    // sentinelģʽ����Ҫ��ɵĳ�ʼ������ 
    initServerConfig();	
    if (server.sentinel_mode) {  
        initSentinelConfig();  
        initSentinel();  
    }  
  
	// ��������
    if (argc >= 2) {     
        loadServerConfig(configfile,options);  
        sdsfree(options);  
    }     
    // ����server���Ӻ�����sentinelTimer 
    initServer();  

    // ���setinel�Ƿ�������׼������configfile��д�� 
    if (!server.sentinel_mode) {  
    } else {  
        sentinelIsRunning();  
    }  

    // ������������ת������   
    aeSetBeforeSleepProc(server.el,beforeSleep);  
    aeMain(server.el);  
    aeDeleteEventLoop(server.el);  
	
    return 0;  
}  

2 �������Ƿ����sentinelģʽ�Լ�sentinelģʽ����Ҫ��ɵĳ�ʼ������
// ����һ��server id
int checkForSentinelMode(int argc, char **argv) {
    int j;

    if (strstr(argv[0],"redis-sentinel") != NULL) return 1;
    for (j = 1; j < argc; j++)
        if (!strcmp(argv[j],"--sentinel")) return 1;
    return 0;
}

/*
 * �������Ϊһ��redis instance����һ��160bit��id
 * (���δ����ʾ������320bit����40����ĸ���ȵĿ���ʾ�ַ���)��
 */
void getRandomHexChars(char *p, unsigned int len)��
{
    char *charset = "0123456789abcdef";
    unsigned int j;

    /* Global state. */
    static int seed_initialized = 0;
    static unsigned char seed[20]; /* The SHA1 seed, from /dev/urandom. */
    static uint64_t counter = 0; /* The counter we hash with the seed. */

    /*  ����һ��������������洢��seed����
    Linux�е���������Դ�����������ļ��в�����һ����/dev/urandom.����һ����/dev/random��
    ���ǲ����������ԭ�������õ�ǰϵͳ���س���������̶�һ��������������أ�Ȼ����Щ������Ϊ�ֽ������ء�
    �سؾ��ǵ�ǰϵͳ�Ļ�����������ָ����һ��ϵͳ�Ļ��ҳ̶ȣ�ϵͳ��������ͨ���ܶ���������������ڴ��ʹ�ã�
    �ļ���ʹ��������ͬ���͵Ľ��������ȵȡ������ǰ���������仯�Ĳ��Ǻܾ��һ��ߵ�ǰ����������С������տ�����ʱ��
    ����ǰ��Ҫ������������أ���ʱ����������������Ч���Ͳ��Ǻܺ��ˡ�

    �����Ϊʲô����/dev/urandom��/dev/random�����ֲ�ͬ���ļ��������ڲ��ܲ����µ������ʱ����������
    ��ǰ�߲��ᣨublock������Ȼ�����������Ч���Ͳ�̫���ˣ���Լ��ܽ���������Ӧ����˵�Ͳ���һ�ֺܺõ�ѡ��
    /dev/random��������ǰ�ĳ���ֱ�������سز����µ�����ֽ�֮��ŷ��أ�����ʹ��/dev/random��ʹ��
    /dev/urandom����������������ٶ�Ҫ���� */
    if (!seed_initialized) {
        FILE *fp = fopen("/dev/urandom", "r");
        if (fp && fread(seed, sizeof(seed), 1, fp) == 1)
            seed_initialized = 1;
        if (fp) fclose(fp);
    }

    if (seed_initialized) {
        // �����/dev/urandom��ȡ��������ַ�����������SHA�㷨����һ��id
        while (len) {
            // len���ܲ�����20����40�ֽڵı�������������Ҫͨ��ѭ����p����������
            unsigned char digest[20];
            SHA1_CTX ctx;
            unsigned int copylen = len > 20 ? 20 : len;

            SHA1Init(&ctx);
            SHA1Update(&ctx, seed, sizeof(seed));
            SHA1Update(&ctx, (unsigned char*)&counter, sizeof(counter));
            SHA1Final(digest, &ctx);
            counter++;

            memcpy(p, digest, copylen);
            /* Convert to hex digits. */
            // ������ת��Ϊ�ɶ��ַ�����ֻ������ֻ����һ���ֽڵĺ�벿��
            for (j = 0; j < copylen; j++) p[j] = charset[p[j] & 0x0F];

            // �ƶ����
            len -= copylen;
            p += copylen;
        }
    }
    else {
        // �����/dev/urandom��ȡ����ַ���ʧ�ܣ�������ʱ��͵�ǰ���̵�id������һ������ַ���
        char *x = p;
        unsigned int l = len;
        struct timeval tv;
        pid_t pid = getpid();

        /* Use time and PID to fill the initial array. */
        // ����buf�����ʱ������΢���������֣�Ȼ���ٲ����Ͻ��̵�id
        gettimeofday(&tv, NULL);
        if (l >= sizeof(tv.tv_usec)) {
            memcpy(x, &tv.tv_usec, sizeof(tv.tv_usec));
            l -= sizeof(tv.tv_usec);
            x += sizeof(tv.tv_usec);
        }
        if (l >= sizeof(tv.tv_sec)) {
            memcpy(x, &tv.tv_sec, sizeof(tv.tv_sec));
            l -= sizeof(tv.tv_sec);
            x += sizeof(tv.tv_sec);
        }
        if (l >= sizeof(pid)) {
            memcpy(x, &pid, sizeof(pid));
            l -= sizeof(pid);
            x += sizeof(pid);
        }
        // �������������������ת��Ϊ16���ƿ����ַ���
        for (j = 0; j < len; j++) {
            p[j] ^= rand();
            p[j] = charset[p[j] & 0x0F];
        }
    }
}
// ��server����һ��id
void initServerConfig(void) {
    // ./redis.h:95:#define REDIS_RUN_ID_SIZE 40
    getRandomHexChars(server.runid,REDIS_RUN_ID_SIZE); 
}
// ��ʼ��server.port
void initSentinelConfig(void) {
    // #define REDIS_SENTINEL_PORT 26379
    server.port = REDIS_SENTINEL_PORT;
}  
// ��ʼ��sentinel����
void initSentinel(void) {
    unsigned int j;

    /* Remove usual Redis commands from the command table, then just add
     * the SENTINEL command. */
	// ��ʼ��sentinel�������ֵ�
    dictEmpty(server.commands,NULL);
    for (j = 0; j < sizeof(sentinelcmds)/sizeof(sentinelcmds[0]); j++) {
        int retval;
        struct redisCommand *cmd = sentinelcmds+j;

        retval = dictAdd(server.commands, sdsnew(cmd->name), cmd);
        redisAssert(retval == DICT_OK);
    }

    /* Initialize various data structures. */
    sentinel.current_epoch = 0;
	// ��ʼ��sentinel��master�ֵ�
    sentinel.masters = dictCreate(&instancesDictType,NULL);
    sentinel.tilt = 0;
    sentinel.tilt_start_time = 0;
    sentinel.previous_time = mstime();
    sentinel.running_scripts = 0;
    sentinel.scripts_queue = listCreate();
    sentinel.announce_ip = NULL;
    sentinel.announce_port = 0;
}

3 ��ʼ������

// ���ļ���ȡ���е��ַ���
void loadServerConfig(char *filename, char *options) {
    sds config = sdsempty();
    char buf[REDIS_CONFIGLINE_MAX+1];

    /* Load the file content */
    if (filename) {
        FILE *fp;

		// ����ļ�����Ϊ�գ����stdin��ȡ
        if (filename[0] == '-' && filename[1] == '\0') {
            fp = stdin;
        } else {
            if ((fp = fopen(filename,"r")) == NULL) {
                redisLog(REDIS_WARNING,
                    "Fatal error, can't open config file '%s'", filename);
                exit(1);
            }
        }
        while(fgets(buf,REDIS_CONFIGLINE_MAX+1,fp) != NULL)
            config = sdscat(config,buf);
        if (fp != stdin) fclose(fp);
    }
    /* Append the additional options */
    if (options) {
        config = sdscat(config,"\n");
        config = sdscat(config,options);
    }
    loadServerConfigFromString(config);
    sdsfree(config);
}
// ���ַ���@config���в��
void loadServerConfigFromString(char *config) {
    char *err = NULL;
    int linenum = 0, totlines, i;
    int slaveof_linenum = 0;
    sds *lines;

	// �����н��зָ�������lines�����У�����Ϊtotlines
    lines = sdssplitlen(config,strlen(config),"\n",1,&totlines);

    for (i = 0; i < totlines; i++) {
        sds *argv;
        int argc;

        linenum = i+1; //��¼�кţ�һ�����������loaderr����˵���������ڵ��к� 
		// ȥ��tab�����С��س��ȿո��
        lines[i] = sdstrim(lines[i]," \t\r\n");
        // ��������к�ע����
        if (lines[i][0] == '#' || lines[i][0] == '\0') continue;

        // ��ÿ���ٽ��зָ�ָ������@argv���飬����elem����Ϊargs
        argv = sdssplitargs(lines[i],&argc);
        if (argv == NULL) { // ����argvΪ�յ����
            err = "Unbalanced quotes in configuration line";
            goto loaderr;
        }
        if (argc == 0) { // ����element numberΪ0�����
            sdsfreesplitres(argv,argc);
            continue;
        }
		sdstolower(argv[0]);  // ��line keyת��Ϊ��Ϣ

        /* Execute config directives */
        if (!strcasecmp(argv[0],"sentinel")) {
            /* argc == 1 is handled by main() as we need to enter the sentinel
             * mode ASAP. */
            if (argc != 1) {
                if (!server.sentinel_mode) {
                    err = "sentinel directive while not in sentinel mode";
                    goto loaderr;
                }
                err = sentinelHandleConfiguration(argv+1,argc-1);
                if (err) goto loaderr;
            }
        } else {
            err = "Bad directive or wrong number of arguments"; goto loaderr;
        }
		// �ͷ�element����
        sdsfreesplitres(argv,argc);
    }

	// �ͷ�line����
	sdsfreesplitres(lines,totlines);
    return;

loaderr:
    fprintf(stderr, "\n*** FATAL CONFIG FILE ERROR ***\n");
    fprintf(stderr, "Reading the configuration file, at line %d\n", linenum);
    fprintf(stderr, ">>> '%s'\n", lines[i]);
    fprintf(stderr, "%s\n", err);
    exit(1);
}

typedef struct sentinelAddr {
    char *ip;
    int port;
} sentinelAddr;
// ����sentinel�ĵ�ַ
sentinelAddr *createSentinelAddr(char *hostname, int port) {
    char ip[REDIS_IP_STR_LEN];
    sentinelAddr *sa;

    if (port <= 0 || port > 65535) {
        errno = EINVAL;
        return NULL;
    }
    if (anetResolve(NULL,hostname,ip,sizeof(ip)) == ANET_ERR) {
        errno = ENOENT;
        return NULL;
    }
    sa = zmalloc(sizeof(*sa));
    sa->ip = sdsnew(ip);
    sa->port = port;
    return sa;
}
// ����һ��sri
/*
 * ����������ڴ���һ��sentinel instance�����ڴ���һ��sentinel�ļ�ػ�����ϵ����
 * ��ϵ���������һ��redis master\redis slave\redis sentinel���������������ڽӵ�info����ʱ�ٸ�ֵ:
 * runid: ��ʼ����ʱ�򱻸�ֵΪnil��
 * info_refresh: ��ʼ����ʱ��ֵΪ0����ʾ��û�нӵ���info���
 *
 * ���flagsֵΪSRI_MASTER����sri�������󣬽�������sentinel��sentinel.masters��ϣ��
 * ���flagsֵΪSRI_SLAVE or SRI_SENTINEL����@name���ã�������hostname:port��Ϊ�Լ���name,
 *        ͬʱ@masterһ������Ϊnil��������sri��������master->slaves or master->sentinels��ϣ��;
 *
 * ���hostname���ܱ���������port������򷵻�nil����errno�ᱻ��Ϊ���ֵ��
 * ���master��ĳ��slave��nameһ�����򷵻�nil��errnoΪEBUSY����Ϊ��ص�hash����name��Ϊhash key��
 */
sentinelRedisInstance *createSentinelRedisInstance(char *name, int flags, char *hostname, int port, int quorum, sentinelRedisInstance *master) {
    sentinelRedisInstance *ri;
    sentinelAddr *addr;
    dict *table = NULL;
    char slavename[128], *sdsname;

    redisAssert(flags & (SRI_MASTER | SRI_SLAVE | SRI_SENTINEL));
    redisAssert((flags & SRI_MASTER) || master != NULL);

    /* Check address validity. */
    addr = createSentinelAddr(hostname, port);
    if (addr == NULL) return NULL;

    /* For slaves and sentinel we use ip:port as name. */
    if (flags & (SRI_SLAVE | SRI_SENTINEL)) {
        snprintf(slavename, sizeof(slavename),
            strchr(hostname, ':') ? "[%s]:%d" : "%s:%d",
            hostname, port);
        name = slavename;
    }

    /* Make sure the entry is not duplicated. This may happen when the same
    * name for a master is used multiple times inside the configuration or
    * if we try to add multiple times a slave or sentinel with same ip/port
    * to a master. */
    // sentinel��ص����е�������sentinel{masters}���棬��ÿ��master��slave
    // �Լ���ص�sentinel����sentinelRedisInstance{slaves, sentinels}����
    if (flags & SRI_MASTER) table = sentinel.masters;
    else if (flags & SRI_SLAVE) table = master->slaves;
    else if (flags & SRI_SENTINEL) table = master->sentinels;
    sdsname = sdsnew(name);
    if (dictFind(table, sdsname)) {
        releaseSentinelAddr(addr);
        sdsfree(sdsname);
        errno = EBUSY;
        return NULL;
    }

    /* Create the instance object. */
    ri = zmalloc(sizeof(*ri));
    /* Note that all the instances are started in the disconnected state,
    * the event loop will take care of connecting them. */
    ri->flags = flags | SRI_DISCONNECTED;
    ri->name = sdsname;
    ri->runid = NULL;
    ri->config_epoch = 0;
    ri->addr = addr;
    ri->cc = NULL;
    ri->pc = NULL;
    ri->pending_commands = 0;
    ri->cc_conn_time = 0;
    ri->pc_conn_time = 0;
    ri->pc_last_activity = 0;
    /* We set the last_ping_time to "now" even if we actually don't have yet
    * a connection with the node, nor we sent a ping.
    * This is useful to detect a timeout in case we'll not be able to connect
    * with the node at all. */
    ri->last_ping_time = mstime();
    ri->last_avail_time = mstime();
    ri->last_pong_time = mstime();
    ri->last_pub_time = mstime();
    ri->last_hello_time = mstime();
    ri->last_master_down_reply_time = mstime();
    ri->s_down_since_time = 0;
    ri->o_down_since_time = 0;
    ri->down_after_period = master ? master->down_after_period :
        SENTINEL_DEFAULT_DOWN_AFTER;
    ri->master_link_down_time = 0;
    ri->auth_pass = NULL;
    ri->slave_priority = SENTINEL_DEFAULT_SLAVE_PRIORITY;
    ri->slave_reconf_sent_time = 0;
    ri->slave_master_host = NULL;
    ri->slave_master_port = 0;
    ri->slave_master_link_status = SENTINEL_MASTER_LINK_STATUS_DOWN;
    ri->slave_repl_offset = 0;
    ri->sentinels = dictCreate(&instancesDictType, NULL);
    ri->quorum = quorum;
    ri->parallel_syncs = SENTINEL_DEFAULT_PARALLEL_SYNCS;
    ri->master = master;
    ri->slaves = dictCreate(&instancesDictType, NULL);
    ri->info_refresh = 0;

    /* Failover state. */
    ri->leader = NULL;
    ri->leader_epoch = 0;
    ri->failover_epoch = 0;
    ri->failover_state = SENTINEL_FAILOVER_STATE_NONE;
    ri->failover_state_change_time = 0;
    ri->failover_start_time = 0;
    ri->failover_timeout = SENTINEL_DEFAULT_FAILOVER_TIMEOUT;
    ri->failover_delay_logged = 0;
    ri->promoted_slave = NULL;
    ri->notification_script = NULL;
    ri->client_reconfig_script = NULL;

    /* Role */
    ri->role_reported = ri->flags & (SRI_MASTER | SRI_SLAVE);
    ri->role_reported_time = mstime();
    ri->slave_conf_change_time = mstime();

    /* Add into the right table. */
    dictAdd(table, ri->name, ri);
    return ri;
}
// ����config��ÿһ�У�����sri
char *sentinelHandleConfiguration(char **argv, int argc) {
    sentinelRedisInstance *ri;

    if (!strcasecmp(argv[0],"monitor") && argc == 5) {
        /* monitor <name> <host> <port> <quorum> */
        int quorum = atoi(argv[4]);

        if (quorum <= 0) return "Quorum must be 1 or greater.";
		// ����monitor sri
        if (createSentinelRedisInstance(argv[1],SRI_MASTER,argv[2],
                                        atoi(argv[3]),quorum,NULL) == NULL)
        {
            switch(errno) {
            case EBUSY: return "Duplicated master name.";
            case ENOENT: return "Can't resolve master instance hostname.";
            case EINVAL: return "Invalid port number";
            }
        }
    } else if (!strcasecmp(argv[0],"down-after-milliseconds") && argc == 3) {
        /* down-after-milliseconds <name> <milliseconds> */
		// ����master��name��ȡsri��Ȼ��������down_after_period��ֵ
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        ri->down_after_period = atoi(argv[2]);
        if (ri->down_after_period <= 0)
            return "negative or zero time parameter.";
        sentinelPropagateDownAfterPeriod(ri);
    } else if (!strcasecmp(argv[0],"failover-timeout") && argc == 3) {
        /* failover-timeout <name> <milliseconds> */
		// ����master��name��ȡsri��Ȼ��������failover-timeout��ֵ
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        ri->failover_timeout = atoi(argv[2]);
        if (ri->failover_timeout <= 0)
            return "negative or zero time parameter.";
   } else if (!strcasecmp(argv[0],"parallel-syncs") && argc == 3) {
        /* parallel-syncs <name> <milliseconds> */
		// ����master��name��ȡsri��Ȼ��������parallel_syncs��ֵ
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        ri->parallel_syncs = atoi(argv[2]);
   } else if (!strcasecmp(argv[0],"notification-script") && argc == 3) {
        /* notification-script <name> <path> */
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        if (access(argv[2],X_OK) == -1)
            return "Notification script seems non existing or non executable.";
        ri->notification_script = sdsnew(argv[2]);
   } else if (!strcasecmp(argv[0],"client-reconfig-script") && argc == 3) {
        /* client-reconfig-script <name> <path> */
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        if (access(argv[2],X_OK) == -1)
            return "Client reconfiguration script seems non existing or "
                   "non executable.";
        ri->client_reconfig_script = sdsnew(argv[2]);
   } else if (!strcasecmp(argv[0],"auth-pass") && argc == 3) {
        /* auth-pass <name> <password> */
		// ����master��name��ȡsri��Ȼ��������auth_pass��ֵ
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        ri->auth_pass = sdsnew(argv[2]);
    } else if (!strcasecmp(argv[0],"current-epoch") && argc == 2) {
        /* current-epoch <epoch> */
        unsigned long long current_epoch = strtoull(argv[1],NULL,10);
        if (current_epoch > sentinel.current_epoch)
            sentinel.current_epoch = current_epoch;
    } else if (!strcasecmp(argv[0],"config-epoch") && argc == 3) {
        /* config-epoch <name> <epoch> */
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        ri->config_epoch = strtoull(argv[2],NULL,10);
        /* The following update of current_epoch is not really useful as
         * now the current epoch is persisted on the config file, but
         * we leave this check here for redundancy. */
        if (ri->config_epoch > sentinel.current_epoch)
            sentinel.current_epoch = ri->config_epoch;
    } else if (!strcasecmp(argv[0],"leader-epoch") && argc == 3) {
        /* leader-epoch <name> <epoch> */
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        ri->leader_epoch = strtoull(argv[2],NULL,10);
    } else if (!strcasecmp(argv[0],"known-slave") && argc == 4) {
        sentinelRedisInstance *slave;

        /* known-slave <name> <ip> <port> */
		// ��redis slave����sri
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        if ((slave = createSentinelRedisInstance(NULL,SRI_SLAVE,argv[2],
                    atoi(argv[3]), ri->quorum, ri)) == NULL)
        {
            return "Wrong hostname or port for slave.";
        }
    } else if (!strcasecmp(argv[0],"known-sentinel") &&
               (argc == 4 || argc == 5)) {
        sentinelRedisInstance *si;

        /* known-sentinel <name> <ip> <port> [runid] */
		// ��sentinel����sri
        ri = sentinelGetMasterByName(argv[1]);
        if (!ri) return "No such master with specified name.";
        if ((si = createSentinelRedisInstance(NULL,SRI_SENTINEL,argv[2],
                    atoi(argv[3]), ri->quorum, ri)) == NULL)
        {
            return "Wrong hostname or port for sentinel.";
        }
        if (argc == 5) si->runid = sdsnew(argv[4]);
    } else if (!strcasecmp(argv[0],"announce-ip") && argc == 2) {
        /* announce-ip <ip-address> */
        if (strlen(argv[1]))
		    // ����sentinel��announce_ip
            sentinel.announce_ip = sdsnew(argv[1]);
    } else if (!strcasecmp(argv[0],"announce-port") && argc == 2) {
        /* announce-port <port> */
		// ����sentinel��announce_port
        sentinel.announce_port = atoi(argv[1]);
    } else {
        return "Unrecognized sentinel configuration statement.";
    }
    return NULL;
}

4 ����server��������ʱ����serverCron

/* Return the UNIX time in microseconds */
long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

/* Return the UNIX time in milliseconds */
long long mstime(void) {
    return ustime()/1000;
}

/* We take a cached value of the unix time in the global state because with
 * virtual memory and aging there is to store the current time in objects at
 * every object access, and accuracy is not needed. To access a global var is
 * a lot faster than calling time(NULL) */
void updateCachedTime(void) {
    server.unixtime = time(NULL);
    server.mstime = mstime();
}

/* Using the following macro you can run code inside serverCron() with the
 * specified period, specified in milliseconds.
 * The actual resolution depends on server.hz. */
// ��һ����ִ��10�Σ�ÿ�ε�interval��100ms
#define REDIS_DEFAULT_HZ        10      /* Time interrupt calls/sec. */ 
// ������У�(1000/server.hz)Ϊһ��������time interval������serverCron��ִ��������100ms���������˺�ĵ�һ���ж�����
// ((_ms_)/(1000/server.hz)) ����ÿ���ٴ�ִ��һ����صļ�飬��ʽ��ЧΪ((_ms_ * server.hz) / (1000)) 
// serverCron����ÿִ��һ�Σ�server.cronloops������һ�Σ�(server.cronloops%((_ms_)/(1000/server.hz))
#define run_with_period(_ms_) if ((_ms_ <= 1000/server.hz) || !(server.cronloops%((_ms_)/(1000/server.hz))))

/* �����redis�Ķ�ʱ��������ÿ1ms�ᱻִ��һ�Σ�һ����ִ��server.hz[10]��
 * �����г���һЩ��ʱ�����б�
 *
 * - ����kv���ռ�ɾ�� (lazyģʽ).
 * - Software watchdog.
 * - ����ͳ��ֵ
 * - �����е�db����rehashing�����и��ؾ���
 * - ����BGSAVE / AOF rewrite, and handling of terminated children.
 * - ɾ�����ڵ�����
 * - Replication�����Ӹ���
 * - ��������
 *
 * �������е�������ִ��ͬ���ļ��Ƶ�ʣ�Ϊ�˶Լ��Ƶ�ʽ��п��ƣ�
 *  ���ﶨ����һ�����������õĺ�run_with_period(milliseconds)
 */
int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    int j;
    REDIS_NOTUSED(eventLoop);
    REDIS_NOTUSED(id);
    REDIS_NOTUSED(clientData);

    /* Software watchdog: deliver the SIGALRM that will reach the signal
     * handler if we don't return here fast enough. */
    if (server.watchdog_period) watchdogScheduleSignal(server.watchdog_period);

    /* Update the time cache. */
    updateCachedTime();

    /* Run the Sentinel timer if we are in sentinel mode. */
	// �����sentinelģʽ�£���ִ��sentinel��ص����������� 
    run_with_period(100) {
        if (server.sentinel_mode) sentinelTimer();
    }

    server.cronloops++;
	// hzĬ��ֵΪ10������sentinel�Ķ�ʱ����sentinelTimer���޸����ֵ��ԭ������̷���6
	// �ο������processTimeEvents���˴�����1000/10 = 100ms�ᵼ��serverCron
    // ����ʱִ��ʱ���ɳ�ʼ��1ms���޸�Ϊ100ms���������Ķ�ʱ����ʱ��Ϊ100ms
    return 1000/server.hz;  
}

// ����event loop�����������˿ڣ���������ʱ����serverCronִ�ж�ʱ���񣬶�ʱ�����1ms
void initServer(void) {
    int j;

	// �źŴ���
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    setupSignalHandlers();

    if (server.syslog_enabled) {
        openlog(server.syslog_ident, LOG_PID | LOG_NDELAY | LOG_NOWAIT,
            server.syslog_facility);
    }

	// ��ʼ����Ҫ��Ա
    server.pid = getpid();
    server.monitors = listCreate();

    createSharedObjects();
    adjustOpenFilesLimit();
	// ����event loop
    server.el = aeCreateEventLoop(server.maxclients+REDIS_EVENTLOOP_FDSET_INCR);
	// ����root db
    server.db = zmalloc(sizeof(redisDb)*server.dbnum);

    // �����˿�
    if (server.port != 0 &&
        listenToPort(server.port,server.ipfd,&server.ipfd_count) == REDIS_ERR)
        exit(1);

    /* Abort if there are no listening sockets at all. */
    if (server.ipfd_count == 0 && server.sofd < 0) {
        redisLog(REDIS_WARNING, "Configured to not listen anywhere, exiting.");
        exit(1);
    }
 
    /* Create the Redis databases, and initialize other internal state. */
	// ����db����
    for (j = 0; j < server.dbnum; j++) {
        server.db[j].dict = dictCreate(&dbDictType,NULL);
    }
    server.pubsub_channels = dictCreate(&keylistDictType,NULL);
    server.pubsub_patterns = listCreate();
    server.dirty = 0;
    resetServerStats();
    /* A few stats we don't want to reset: server startup time, and peak mem. */
    server.stat_starttime = time(NULL);
    updateCachedTime();

    /* Create the serverCron() time event, that's our main way to process
     * background operations. */
	// ������ʱ�� 
    if(aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL) == AE_ERR) {  
        redisPanic("Can't create the serverCron time event.");
        exit(1);
    }

    /* Create an event handler for accepting new connections in TCP and Unix
     * domain sockets. */
    for (j = 0; j < server.ipfd_count; j++) {
        printf("initServer invoke acceptTcpHandler\n");
        if (aeCreateFileEvent(server.el, server.ipfd[j], AE_READABLE,
            acceptTcpHandler,NULL) == AE_ERR)
            {
                redisPanic(
                    "Unrecoverable error creating server.ipfd file event.");
            }
    }
}

!!!!ע�⣺����initServer�г�ʼע�ᶨʱ������serverCron��ʱ���õ�ʱ������1ms��������������
���Ǻ����ٴε���ʱ��ʱ�����Ǻ����ķ���ֵ��
/* Process time events */
static int processTimeEvents(aeEventLoop *eventLoop) {
    int processed = 0;
    aeTimeEvent *te;
    long long maxId;
    time_t now = time(NULL);
	
	te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId-1;
    while(te) {
        long now_sec, now_ms;
        long long id;

        if (te->id > maxId) {
            te = te->next;
            continue;
        }
        aeGetTime(&now_sec, &now_ms);
        if (now_sec > te->when_sec ||
            (now_sec == te->when_sec && now_ms >= te->when_ms))
        {   
            int retval;
            
            id = te->id;
            retval = te->timeProc(eventLoop, id, te->clientData);
            processed++;

            if (retval != AE_NOMORE) {  // ����retval���޸Ķ�ʱ���������ִ��ʱ��@te->when_sec&@te->when_ms
                aeAddMillisecondsToNow(retval,&te->when_sec,&te->when_ms);
            } else {  // �������ֵ��-1����ɾ����ʱ����
                aeDeleteTimeEvent(eventLoop, id);
            }
			te = eventLoop->timeEventHead;
        } else {
            te = te->next;
        }
    }
    return processed;
}
			
5 ���������

// ���configfile�Ƿ�����Լ��Ƿ��д
void sentinelIsRunning(void) {
    redisLog(REDIS_WARNING,"Sentinel runid is %s", server.runid);

    if (server.configfile == NULL) {
		// ���configfile�Ƿ����
		// configfile�������л�sentinel�ļ������
        redisLog(REDIS_WARNING,
            "Sentinel started without a config file. Exiting...");
        exit(1);
    } else if (access(server.configfile,W_OK) == -1) {
	    // ���configfile�Ƿ��д
        redisLog(REDIS_WARNING,
            "Sentinel config file %s is not writable: %s. Exiting...",
            server.configfile,strerror(errno));
        exit(1);
    }

    /* We want to generate a +monitor event for every configured master
     * at startup. */
    sentinelGenerateInitialMonitorEvents();
}

/* ��¼event log��ִ��pub/sub, ִ��notification�ű�
 *
 * 'level'��log level��ֻ��log level��REDIS_WARNINGʱ���Ż�ִ��notification�ű�
 * 'type'��message type, Ҳ����pub/sub channel name.
 * 'ri'��sri�����Ի�ȡ��path of the notification script
 * 'fmt' printf-alike.
 *       ���fmt��"%@"��ͷ����ri����NULL����log���ݵĿ�ͷ�����¸�ʽչ����
 *       <instance type> <instance name> <ip> <port>
 *       ���@ri����master�������Ҫ������mmaster����Ϣ:
 *  	 @ <master name> <master ip> <master port>
 *       ʣ�ಿ�ֵĴ����printf����
 */
void sentinelEvent(int level, char *type, sentinelRedisInstance *ri,
                   const char *fmt, ...) {
    va_list ap;
    char msg[REDIS_MAX_LOGMSG_LEN];  // log�����1024B
    robj *channel, *payload;

    /* Handle %@ */
	// ƴдmsg pre
    if (fmt[0] == '%' && fmt[1] == '@') {
        sentinelRedisInstance *master = (ri->flags & SRI_MASTER) ?
                                         NULL : ri->master;

        if (master) {
            snprintf(msg, sizeof(msg), "%s %s %s %d @ %s %s %d",
                sentinelRedisInstanceTypeStr(ri),
                ri->name, ri->addr->ip, ri->addr->port,
                master->name, master->addr->ip, master->addr->port);
        } else {
            snprintf(msg, sizeof(msg), "%s %s %s %d",
                sentinelRedisInstanceTypeStr(ri),
                ri->name, ri->addr->ip, ri->addr->port);
        }
        fmt += 2;
    } else {
        msg[0] = '\0';
    }

    /* Use vsprintf for the rest of the formatting if any. */
	// ƴ��msg����
    if (fmt[0] != '\0') {
        va_start(ap, fmt);
        vsnprintf(msg+strlen(msg), sizeof(msg)-strlen(msg), fmt, ap);
        va_end(ap);
    }

    /* Log the message if the log level allows it to be logged. */
	// ��¼log
    if (level >= server.verbosity)
        redisLog(level,"%s %s",type,msg);

    /* Publish the message via Pub/Sub if it's not a debugging one. */
	// ִ��pub/sub�����緢����failoverʱ����hello channel����֪ͨ
    if (level != REDIS_DEBUG) {
        channel = createStringObject(type,strlen(type));  // type��channel
        payload = createStringObject(msg,strlen(msg));    // msg����
        pubsubPublishMessage(channel,payload); // pubsub
        decrRefCount(channel);
        decrRefCount(payload);
    }

    /* Call the notification script if applicable. */
	// ִ��notify script�����緢����failoverʱ�����ִ��һЩscript
    if (level == REDIS_WARNING && ri != NULL) {
        sentinelRedisInstance *master = (ri->flags & SRI_MASTER) ?
                                         ri : ri->master;
        if (master->notification_script) {
            sentinelScheduleScriptExecution(master->notification_script,
                type,msg,NULL); // notify script
        }
    }
}

/* This function is called only at startup and is used to generate a
 * +monitor event for every configured master. The same events are also
 * generated when a master to monitor is added at runtime via the
 * SENTINEL MONITOR command. */
void sentinelGenerateInitialMonitorEvents(void) {
    dictIterator *di;
    dictEntry *de;

    di = dictGetIterator(sentinel.masters);
    while((de = dictNext(di)) != NULL) {
	    // ��ȡʵ��
        sentinelRedisInstance *ri = dictGetVal(de);
		// ���ÿ��ʵ������ʵ����Ϣlog����
        sentinelEvent(REDIS_WARNING,"+monitor",ri,"%@ quorum %d",ri->quorum);
    }
    dictReleaseIterator(di);
}

6 sentinel�Ķ�ʱ��������
	����������̷�����Ҫ˵����sentinel���������е�ִ�в��裬�������sentinel����֮������̡�
	
void sentinelTimer(void) {
    sentinelCheckTiltCondition();
    sentinelHandleDictOfRedisInstances(sentinel.masters);
    sentinelRunPendingScripts();
    sentinelCollectTerminatedScripts();
    sentinelKillTimedoutScripts();

    /* We continuously change the frequency of the Redis "timer interrupt"
     * in order to desynchronize every Sentinel from every other.
     * This non-determinism avoids that Sentinels started at the same time
     * exactly continue to stay synchronized asking to be voted at the
     * same time again and again (resulting in nobody likely winning the
     * election because of split brain voting). */
    server.hz = REDIS_DEFAULT_HZ + rand() % REDIS_DEFAULT_HZ;
}

6.1 tiltģʽ
/* ������������ж��Ƿ����tiltģʽ
*
* �������sentinelTimer���õ�ʱ����̫�ࣨSENTINEL_TILT_TRIGGER = 2s������
* ʱ�䷢�����ң���ǰʱ����ϴ�ʱ��С����ô�ͽ���tiltģʽ�� ������Щ����Ŀ�
* ��ԭ�����£�
*
* 1) Sentiel��������һЩԭ������ס�ˣ���load���ļ�̫������һЩI/O����
* ԭ��os��Ӧ�ٶȼ����������յ�һЩ�źź�stopס�ȵȡ�
* 2) ϵͳ��ʱ�ӱ������ˡ�
*
* ��������Դ�����ôredis�����е�timer�¼����ᳬʱ���ߴ���ʧ�ܡ�redis��
* ����ʽ���ǽ���tiltģʽ�������ռ�ϵͳ��Ϣ�������ⲻ�ٴ����κ�����(����
* ����һ��)��ֱ��tiltģʽ��ʱ(SENTINEL_TILT_PERIOD = 30s)Ϊֹ��
*/
void sentinelCheckTiltCondition(void) {
	mstime_t now = mstime();
	mstime_t delta = now - sentinel.previous_time;

	if (delta < 0 || delta > SENTINEL_TILT_TRIGGER) {
		sentinel.tilt = 1;
		sentinel.tilt_start_time = mstime();
		// ����tiltģʽ����+tilt channel�Ϸ���֪ͨ
		sentinelEvent(REDIS_WARNING, "+tilt", NULL, "#tilt mode entered");
	}
	sentinel.previous_time = mstime();
}
/*
*��sentinel��timer������sentinelHandleRedisInstance�У��ж�tiltģʽ�Ƿ�ʱ
*/
void sentinelHandleRedisInstance(sentinelRedisInstance *ri) {
	/* ============== ACTING HALF ============= */
	/* We don't proceed with the acting half if we are in TILT mode.
	* TILT happens when we find something odd with the time, like a
	* sudden change in the clock. */
	if (sentinel.tilt) {
		if (mstime() - sentinel.tilt_start_time < SENTINEL_TILT_PERIOD) return;
		sentinel.tilt = 0;
		sentinelEvent(REDIS_WARNING, "-tilt", NULL, "#tilt mode exited");
	}
}

��sentinel tiltģʽ�£�����������������Ϊ�ܵ���Ӱ�죺
/* 
 *���ܽ���master�Ƿ����down״̬���ж�
 */
void sentinelCommand(redisClient *c) {
	if (!strcasecmp(c->argv[1]->ptr, "is-master-down-by-addr")) {
		/* SENTINEL IS-MASTER-DOWN-BY-ADDR <ip> <port> <current-epoch> <runid>*/
		int isdown = 0;
		ri = getSentinelRedisInstanceByAddrAndRunID(sentinel.masters,
			c->argv[2]->ptr, port, NULL);

		/* It exists? Is actually a master? Is subjectively down? It's down.
		 * Note: if we are in tilt mode we always reply with "0". */
		if (!sentinel.tilt && ri && (ri->flags & SRI_S_DOWN) &&
			(ri->flags & SRI_MASTER))
			isdown = 1;
	}
}

/*
 * ���ܽ���master��slave״̬���л�
 * ���ܽ���slave��master״̬���л�
 * ���ܶ�slave�任�µ�master
 * ������slave����ͻ��˷�����config����
 */
void sentinelRefreshInstanceInfo(sentinelRedisInstance *ri, const char *info) {
	/* Process line by line. */
	lines = sdssplitlen(info, strlen(info), "\r\n", 2, &numlines);
	for (j = 0; j < numlines; j++) {
		/* None of the following conditions are processed when in tilt mode, so
		 * return asap. */
		if (sentinel.tilt) return;

		/* Handle master -> slave role switch. */
		/* Handle slave -> master role switch. */
		/* Handle slaves replicating to a different master address. */
		/* Detect if the slave that is in the process of being reconfigured
		 * changed state. */
	}
}

6.2 ������е�redis instance��״̬

/* Perform scheduled operations for all the instances in the dictionary.
* Recursively call the function against dictionaries of slaves. */
void sentinelHandleDictOfRedisInstances(dict *instances) {
	dictIterator *di;
	dictEntry *de;
	sentinelRedisInstance *switch_to_promoted = NULL;

	/* There are a number of things we need to perform against every master. */
	di = dictGetIterator(instances);
	while ((de = dictNext(di)) != NULL) {
		sentinelRedisInstance *ri = dictGetVal(de);

		sentinelHandleRedisInstance(ri);
		if (ri->flags & SRI_MASTER) {
			sentinelHandleDictOfRedisInstances(ri->slaves);
			sentinelHandleDictOfRedisInstances(ri->sentinels);
			if (ri->failover_state == SENTINEL_FAILOVER_STATE_UPDATE_CONFIG) {
				switch_to_promoted = ri;
			}
		}
	}
	if (switch_to_promoted)
		sentinelFailoverSwitchToPromotedSlave(switch_to_promoted);
	dictReleaseIterator(di);
}


��Ҫ�ο��ĵ���
1 redis/src/sentinel.c
2 http://blog.csdn.net/yfkiss/article/details/22151175
3 http://blog.csdn.net/yfkiss/article/details/22687771
4 http://redisdoc.com/topic/sentinel.html