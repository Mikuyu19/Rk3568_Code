/**
 * @file lv_ime_pinyin.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_ime_pinyin.h"
#if LV_USE_IME_PINYIN != 0

#include <stdio.h>
#include "../../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_ime_pinyin_class
#define cand_len LV_GLOBAL_DEFAULT()->ime_cand_len

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_ime_pinyin_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_ime_pinyin_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
static void lv_ime_pinyin_style_change_event(lv_event_t *e);
static void lv_ime_pinyin_kb_event(lv_event_t *e);
static void lv_ime_pinyin_cand_panel_event(lv_event_t *e);

static void init_pinyin_dict(lv_obj_t *obj, const lv_pinyin_dict_t *dict);
static void pinyin_input_proc(lv_obj_t *obj);
static void pinyin_page_proc(lv_obj_t *obj, uint16_t btn);
static char *pinyin_search_matching(lv_obj_t *obj, char *py_str, uint16_t *cand_num);
static void pinyin_ime_clear_data(lv_obj_t *obj);

#if LV_IME_PINYIN_USE_K9_MODE
static void pinyin_k9_init_data(lv_obj_t *obj);
static void pinyin_k9_get_legal_py(lv_obj_t *obj, char *k9_input, const char *py9_map[]);
static bool pinyin_k9_is_valid_py(lv_obj_t *obj, char *py_str);
static void pinyin_k9_fill_cand(lv_obj_t *obj);
static void pinyin_k9_cand_page_proc(lv_obj_t *obj, uint16_t dir);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_ime_pinyin_class = {
    .constructor_cb = lv_ime_pinyin_constructor,
    .destructor_cb = lv_ime_pinyin_destructor,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_ime_pinyin_t),
    .base_class = &lv_obj_class,
    .name = "ime-pinyin",
};

#if LV_IME_PINYIN_USE_K9_MODE
static const char *lv_btnm_def_pinyin_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21] = {
    ",\0", "123\0", "abc \0", "def\0", LV_SYMBOL_BACKSPACE "\0", "\n\0",
    ".\0", "ghi\0", "jkl\0", "mno\0", LV_SYMBOL_KEYBOARD "\0", "\n\0",
    "?\0", "pqrs\0", "tuv\0", "wxyz\0", LV_SYMBOL_NEW_LINE "\0", "\n\0",
    LV_SYMBOL_LEFT "\0", "\0"};

static lv_buttonmatrix_ctrl_t default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 17] = {1};
static char lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 2][LV_IME_PINYIN_K9_MAX_INPUT] = {0};
#endif

static char lv_pinyin_cand_str[LV_IME_PINYIN_CAND_TEXT_NUM][4];
static char *lv_btnm_def_pinyin_sel_map[LV_IME_PINYIN_CAND_TEXT_NUM + 3];

#if LV_IME_PINYIN_USE_DEFAULT_DICT
static const lv_pinyin_dict_t lv_ime_pinyin_def_dict[] = {
    {"a", "啊阿吖嗄腌锕錒呵腌"},
    {"ai", "爱哎唉埃挨矮艾碍癌哀蔼"},
    {"an", "安按暗岸案俺鞍氨庵胺鹌"},
    {"ang", "昂肮盎腌骯醠枊"},
    {"ao", "奥熬傲凹澳懊翱袄敖鳌"},
    {"ba", "吧把八爸巴拔罢霸坝芭疤"},
    {"bai", "白百摆败拜柏掰佰呗稗捭"},
    {"ban", "班搬板办半般版伴扮拌颁"},
    {"bang", "帮棒绑榜膀磅傍邦浜谤"},
    {"bao", "包报保抱宝暴饱薄爆豹"},
    {"bei", "北被备背杯悲碑贝辈倍卑"},
    {"ben", "本奔笨苯夯贲锛畚坌"},
    {"beng", "蹦绷甭泵崩迸嘣蚌甏"},
    {"bi", "比笔必逼鼻闭毕碧避币彼"},
    {"bian", "边变便编遍辩扁鞭辨贬"},
    {"biao", "表标彪膘镖裱杓飑骠"},
    {"bie", "别憋瘪蹩鳖彆"},
    {"bin", "宾彬滨濒殡鬓缤斌槟"},
    {"bing", "冰病兵并饼丙柄秉屏禀"},
    {"bo", "波播伯博剥拨薄勃驳玻"},
    {"bu", "不步部补布捕卜怖哺簿"},
    {"ca", "擦嚓礤"},
    {"cai", "才菜采财猜踩裁材彩睬"},
    {"can", "参残餐惨蚕灿惭掺璨"},
    {"cang", "藏仓苍舱沧伧"},
    {"cao", "草操曹糙嘈槽漕艹"},
    {"ce", "测策侧册厕恻"},
    {"cen", "岑涔"},
    {"ceng", "层曾蹭噌"},
    {"cha", "茶查差插察叉茬碴搽"},
    {"chai", "柴拆差豺钗侪虿"},
    {"chan", "产缠馋禅蝉铲掺阐颤"},
    {"chang", "长常厂唱场尝肠昌敞畅"},
    {"chao", "超吵抄朝潮炒钞巢嘲"},
    {"che", "车扯撤彻澈掣"},
    {"chen", "晨陈沉尘臣趁衬辰琛"},
    {"cheng", "成城程称乘盛诚承撑秤"},
    {"chi", "吃迟尺池痴持赤齿耻"},
    {"chong", "冲重虫充宠崇铳舂"},
    {"chou", "抽愁丑臭仇筹绸酬瞅"},
    {"chu", "出处初除触楚厨储畜"},
    {"chuai", "揣踹啜"},
    {"chuan", "传船穿串喘川椽钏"},
    {"chuang", "窗床创闯疮幢"},
    {"chui", "吹垂锤炊陲槌"},
    {"chun", "春纯唇蠢醇淳椿"},
    {"chuo", "戳绰啜辍龊"},
    {"ci", "词次此刺瓷辞慈磁赐"},
    {"cong", "从匆聪葱丛囱淙琮"},
    {"cou", "凑辏腠"},
    {"cu", "粗促醋簇猝蹙蹴"},
    {"cuan", "窜篡蹿攒汆"},
    {"cui", "催脆崔粹摧翠悴淬"},
    {"cun", "村存寸蹲忖皴"},
    {"cuo", "错搓挫措厝磋蹉"},
    {"da", "大答打达搭瘩哒嗒沓"},
    {"dai", "代带待袋戴呆贷歹逮"},
    {"dan", "但单蛋担淡胆弹丹耽"},
    {"dang", "当党荡档挡铛裆"},
    {"dao", "到道倒岛刀盗稻导捣"},
    {"de", "的得德地底"},
    {"dei", "得"},
    {"deng", "等灯登邓瞪凳蹬"},
    {"di", "地第低底弟敌滴帝递"},
    {"dian", "点电店典颠垫殿滇甸"},
    {"diao", "调掉吊雕叼碉钓"},
    {"die", "爹跌叠碟蝶谍迭"},
    {"ding", "定顶盯订丁鼎叮钉"},
    {"diu", "丢铥"},
    {"dong", "东动冬懂洞冻栋董"},
    {"dou", "都斗豆抖兜逗陡痘"},
    {"du", "读度独毒堵肚杜赌"},
    {"duan", "短段断端锻缎"},
    {"dui", "对队堆兑怼碓"},
    {"dun", "顿蹲盾吨钝炖遁"},
    {"duo", "多夺朵躲舵剁跺堕"},
    {"e", "饿额鹅恶俄哦蛾扼"},
    {"ei", "诶"},
    {"en", "恩摁蒽"},
    {"er", "儿二而耳尔饵贰迩"},
    {"fa", "发法罚乏伐阀筏珐"},
    {"fan", "反饭翻凡烦犯范繁返"},
    {"fang", "方放房访防芳仿妨纺"},
    {"fei", "飞非费肥废肺啡菲妃"},
    {"fen", "分份粉奋愤纷坟芬氛"},
    {"feng", "风封疯丰峰缝凤奉枫"},
    {"fo", "佛"},
    {"fou", "否缶"},
    {"fu", "父服福富夫复付负附"},
    {"ga", "嘎尬咖噶轧伽旮"},
    {"gai", "该改盖概钙丐溉垓"},
    {"gan", "干感赶敢甘肝杆竿柑"},
    {"gang", "刚钢港岗纲缸杠肛"},
    {"gao", "高搞告稿糕膏镐皋"},
    {"ge", "哥个歌格割隔革葛戈"},
    {"gei", "给"},
    {"gen", "跟根艮哏"},
    {"geng", "更耕耿梗庚羹埂"},
    {"gong", "工公共供功攻宫弓恭"},
    {"gou", "狗够勾购沟构苟钩垢"},
    {"gu", "古故顾股骨鼓谷固孤"},
    {"gua", "瓜挂刮寡呱卦褂"},
    {"guai", "怪乖拐掴"},
    {"guan", "关管官观馆惯冠灌"},
    {"guang", "光广逛咣胱"},
    {"gui", "归贵鬼规跪柜轨桂龟"},
    {"gun", "滚棍辊衮"},
    {"guo", "国过果锅郭裹帼"},
    // { "ha", "哈蛤虾呵" },
    {"hai", "还海害孩嗨亥骇氦骸"},
    {"han", "汉喊含寒汗韩旱函罕"},
    {"hang", "行航巷杭夯吭沆绗"},
    {"hao", "好号豪毫耗浩郝嚎昊"},
    {"he", "和喝合河何核贺荷赫"},
    {"hei", "黑嘿嗨"},
    {"hen", "很恨狠痕"},
    {"heng", "横恒哼衡亨桁珩"},
    {"hong", "红轰哄洪宏虹鸿弘泓"},
    {"hou", "后候厚猴喉吼侯逅"},
    {"hu", "湖户呼虎胡护互忽狐"},
    {"hua", "话花化画华滑划哗桦"},
    {"huai", "坏怀划槐徊淮"},
    {"huan", "换还欢环患缓唤幻焕"},
    {"huang", "黄慌晃荒皇凰谎煌"},
    {"hui", "会回灰挥汇辉毁惠悔"},
    {"hun", "混昏婚魂浑荤诨"},
    {"huo", "或活火伙货获祸惑霍"},
    {"i", ""},
    {"ji", "几及急既即机鸡积记"},
    {"jia", "家加假价架甲佳夹嘉"},
    {"jian", "见件减间建检简坚健"},
    {"jiang", "将讲江奖降浆僵匠"},
    {"jiao", "叫教交较觉角脚焦胶"},
    {"jie", "接节街结解界姐阶借"},
    {"jin", "进今近金仅紧尽劲斤"},
    {"jing", "经精京静境景竟惊净"},
    {"jiong", "窘炯迥"},
    {"jiu", "就九酒旧久揪救纠舅"},
    {"ju", "句举具局居剧巨聚拒"},
    {"juan", "卷捐娟倦眷绢鹃"},
    {"jue", "觉决绝爵掘诀倔角"},
    {"jun", "军君均菌俊峻骏钧"},
    {"ka", "卡咖咯咔喀佧"},
    {"kai", "开凯慨楷揩恺铠"},
    {"kan", "看砍刊堪坎侃槛勘"},
    {"kang", "抗康扛炕亢慷糠"},
    {"kao", "考靠烤拷犒栲"},
    {"ke", "可克科客刻课颗壳柯"},
    {"ken", "肯啃恳垦"},
    {"keng", "坑吭铿"},
    {"kong", "空恐控孔崆"},
    {"kou", "口扣抠寇叩"},
    {"ku", "苦哭库裤酷枯窟骷"},
    {"kua", "夸跨垮挎胯"},
    {"kuai", "快块筷会侩蒯"},
    {"kuan", "宽款髋"},
    {"kuang", "况狂矿框旷筐眶"},
    {"kui", "亏愧奎溃葵窥魁馈"},
    {"kun", "困昆捆坤悃"},
    {"kuo", "阔扩括廓"},
    {"la", "拉啦辣蜡腊喇垃"},
    {"lai", "来赖莱睐癞籁"},
    {"lan", "蓝兰烂懒栏拦篮览澜"},
    {"lang", "浪狼朗郎廊琅榔"},
    {"lao", "老劳牢捞唠烙涝"},
    {"le", "了乐勒肋叻"},
    {"lei", "类累雷泪勒垒磊蕾"},
    {"leng", "冷愣棱楞"},
    {"li", "例梨里离力理利李立历丽"},
    {"lia", "俩"},
    {"lian", "连脸练联恋莲链怜廉"},
    {"liang", "两量亮良凉辆梁粮"},
    {"liao", "了料聊疗辽僚寥撩"},
    {"lie", "列烈裂猎劣咧冽"},
    {"lin", "林临邻淋琳磷鳞凛"},
    {"ling", "领另零灵令铃陵岭凌"},
    {"liu", "六流留刘柳溜榴琉"},
    {"lo", "咯"},
    {"long", "龙弄隆笼聋垄拢陇"},
    {"lou", "楼漏搂陋娄篓"},
    {"lu", "路露陆录鹿炉卢鲁碌"},
    {"luan", "乱卵峦挛孪"},
    {"lun", "论轮伦沦仑纶"},
    {"luo", "落罗洛络逻萝裸骆"},
    {"lv", "绿率律旅虑驴吕铝"},
    {"lve", "略掠"},
    {"ma", "吗马妈麻骂码嘛玛"},
    {"mai", "买卖麦埋迈脉霾"},
    {"man", "满慢漫蛮瞒蔓曼"},
    {"mang", "忙盲茫芒氓莽"},
    {"mao", "猫毛冒帽矛貌茂贸"},
    {"me", "么麽"},
    {"mei", "没美每妹梅眉媒枚"},
    {"men", "们门闷扪"},
    {"meng", "梦猛蒙盟萌孟朦"},
    {"mi", "米密迷眯蜜秘弥靡"},
    {"mian", "面免棉眠缅勉冕"},
    {"miao", "秒妙苗描庙瞄渺"},
    {"mie", "灭蔑咩"},
    {"min", "民敏抿皿悯闽"},
    {"ming", "名明命鸣铭冥茗"},
    {"miu", "谬缪"},
    {"mo", "摸磨末莫默魔模摩"},
    {"mou", "某谋眸牟缪"},
    {"mu", "目母木幕慕牧墓姆"},
    {"na", "那拿哪纳娜呐捺"},
    {"nai", "奶耐乃奈氖"},
    {"nan", "男难南喃楠囡"},
    {"nang", "囊囔馕"},
    {"nao", "闹脑恼挠淖孬"},
    {"ne", "呢讷"},
    {"nei", "内那馁"},
    {"nen", "嫩恁"},
    {"neng", "能"},
    {"ni", "你泥尼逆妮匿腻拟"},
    {"nian", "年念粘碾捻蔫"},
    {"niang", "娘酿"},
    {"niao", "鸟尿袅"},
    {"nie", "捏聂孽涅啮"},
    {"nin", "您"},
    {"ning", "宁凝拧狞泞"},
    {"niu", "牛扭纽钮拗"},
    {"nong", "农弄浓脓侬"},
    {"nou", "耨"},
    {"nu", "怒奴努弩驽"},
    {"nuan", "暖"},
    {"nue", "虐疟"},
    {"nuo", "挪诺糯懦"},
    {"o", "哦喔噢"},
    {"ou", "偶欧呕殴鸥藕"},
    {"pa", "怕爬趴啪帕琶葩"},
    {"pai", "派排拍牌徘湃"},
    {"pan", "盘判盼叛攀畔潘"},
    {"pang", "旁胖庞乓膀磅"},
    {"pao", "跑炮抛泡袍刨咆"},
    {"pei", "配陪赔佩培呸沛"},
    {"pen", "喷盆湓"},
    {"peng", "碰朋捧棚鹏烹彭"},
    {"pi", "皮批屁披匹劈疲僻"},
    {"pian", "片篇骗偏便翩"},
    {"piao", "票漂飘瓢嫖瞟"},
    {"pie", "撇瞥"},
    {"pin", "品贫拼频聘拚姘"},
    {"ping", "苹平评瓶凭屏乒萍"},
    {"po", "破坡婆迫泼颇泊魄"},
    {"pou", "剖"},
    {"pu", "普扑铺谱朴葡蒲"},
    {"qi", "起其七气期齐奇妻骑"},
    {"qia", "恰掐洽"},
    {"qian", "前钱千签浅牵欠铅"},
    {"qiang", "强枪墙抢腔呛羌"},
    {"qiao", "桥瞧乔巧敲翘俏窍"},
    {"qie", "切且窃茄怯妾"},
    {"qin", "亲琴勤侵秦钦芹寝"},
    {"qing", "情请清青轻晴庆倾"},
    {"qiong", "穷琼穹"},
    {"qiu", "求球秋丘囚酋泅"},
    {"qu", "去取区曲趣屈渠趋"},
    {"quan", "全权圈劝泉拳犬券"},
    {"que", "却确缺雀瘸鹊"},
    {"qun", "群裙逡"},
    {"ran", "然染燃冉苒"},
    {"rang", "让嚷壤瓤"},
    {"rao", "绕扰饶娆"},
    {"re", "热惹"},
    {"ren", "人认任忍仁韧妊"},
    {"reng", "仍扔"},
    {"ri", "日"},
    {"rong", "容荣融绒溶蓉熔"},
    {"rou", "肉柔揉蹂"},
    {"ru", "如入辱乳儒蠕汝"},
    {"ruan", "软阮"},
    {"rui", "瑞锐蕊芮"},
    {"run", "润闰"},
    {"ruo", "若弱偌"},
    {"sa", "撒洒萨卅"},
    {"sai", "塞赛腮鳃"},
    {"san", "三散伞叁"},
    {"sang", "桑丧嗓"},
    {"sao", "扫骚嫂搔"},
    {"se", "色塞瑟涩"},
    {"sen", "森"},
    {"seng", "僧"},
    {"sha", "杀沙啥傻厦纱刹"},
    {"shai", "晒筛"},
    {"shan", "山闪善衫删扇陕"},
    {"shang", "上商伤尚赏裳"},
    {"shao", "少烧稍勺绍哨"},
    {"she", "社设射蛇舍舌涉"},
    {"shen", "身深神什伸甚沈"},
    {"sheng", "生声省胜升盛剩"},
    {"shi", "市食师是时释十使事实式识"},
    {"shou", "手受收首守瘦售"},
    {"shu", "书数树属术熟输"},
    {"shua", "刷耍"},
    {"shuai", "摔甩帅衰"},
    {"shuan", "栓拴"},
    {"shuang", "双爽霜"},
    {"shui", "水谁睡税"},
    {"shun", "顺瞬吮"},
    {"shuo", "说硕烁"},
    {"si", "四死思司似私丝斯"},
    {"song", "送松宋耸诵讼"},
    {"sou", "搜艘嗽嗖"},
    {"su", "速素苏诉俗肃宿"},
    {"suan", "算酸蒜"},
    {"sui", "岁随虽碎穗遂髓"},
    {"sun", "孙损笋"},
    {"suo", "所缩锁索嗦梭"},
    {"ta", "他她它踏塔塌榻"},
    {"tai", "太台态抬泰胎苔汰"},
    {"tan", "谈叹探弹坦摊贪潭"},
    {"tang", "堂汤唐躺糖趟塘"},
    {"tao", "套讨逃桃陶涛淘"},
    {"te", "特忑"},
    {"teng", "疼腾藤誊"},
    {"ti", "体提题替踢梯涕啼"},
    {"tian", "天田甜添填舔恬"},
    {"tiao", "条调跳挑迢眺"},
    {"tie", "铁贴帖"},
    {"ting", "听停庭挺厅亭婷"},
    {"tong", "同通痛童统铜桶"},
    {"tou", "头投偷透"},
    {"tu", "图土突途徒吐涂"},
    {"tuan", "团湍"},
    {"tui", "推退腿褪"},
    {"tun", "吞屯臀"},
    {"tuo", "脱托拖妥拓陀驼"},
    {"u", ""},
    {"v", ""},
    {"wa", "哇挖瓦娃袜娲"},
    {"wai", "外歪崴"},
    {"wan", "完万晚玩碗弯湾丸"},
    {"wang", "往王望忘网旺亡妄"},
    {"wei", "为位未委维味围伟"},
    {"wen", "问文温闻稳吻纹"},
    {"weng", "翁嗡瓮"},
    {"wo", "我握窝卧沃涡斡"},
    {"wu", "无五物务武午舞屋"},
    {"xi", "西系息喜洗细习吸"},
    {"xia", "下夏吓瞎虾峡霞"},
    {"xian", "现先线显县闲险限"},
    {"xiang", "想相向象香乡像响"},
    {"xiao", "小笑校消晓效萧削"},
    {"xie", "些写谢协鞋斜歇"},
    {"xin", "心新信欣辛薪芯馨"},
    {"xing", "行性星兴形醒姓幸"},
    {"xiong", "雄兄凶胸熊匈"},
    {"xiu", "修秀休袖羞朽嗅"},
    {"xu", "许续需须徐序虚蓄"},
    {"xuan", "选宣旋悬玄炫轩"},
    {"xue", "学血雪削穴靴"},
    {"xun", "讯训寻迅询循熏"},
    {"ya", "呀压牙雅亚鸭丫崖"},
    {"yan", "眼言严演烟沿研颜"},
    {"yang", "样阳养羊洋央扬仰"},
    {"yao", "要药摇咬腰妖邀"},
    {"ye", "也夜业叶爷野液页"},
    {"yi", "一以已意议义医衣"},
    {"yin", "因音引印银隐饮阴"},
    {"ying", "应影英营硬迎赢鹰"},
    {"yo", "哟唷"},
    {"yong", "用永勇拥泳庸涌"},
    {"you", "有又由友右油游优"},
    {"yu", "于与语育余遇雨鱼"},
    {"yuan", "远原员元院圆愿源"},
    {"yue", "月越约乐阅跃岳"},
    {"yun", "云运允晕韵孕匀"},
    {"za", "杂砸咋扎"},
    {"zai", "在再载灾仔宰哉"},
    {"zan", "咱暂攒赞"},
    {"zang", "脏藏葬赃"},
    {"zao", "早造遭澡燥糟灶"},
    {"ze", "则责择泽咋仄"},
    {"zei", "贼"},
    {"zen", "怎谮"},
    {"zeng", "增曾赠憎"},
    {"zha", "炸扎渣闸眨榨喳"},
    {"zhai", "债宅摘窄斋"},
    {"zhan", "站展战占沾斩盏"},
    {"zhang", "长张章掌涨丈障"},
    {"zhao", "找照招朝赵兆罩"},
    {"zhe", "这着者折哲遮辙"},
    {"zhen", "真阵镇针震珍诊"},
    {"zheng", "拯正政证争整征症"},
    {"zhi", "之知只制直治指"},
    {"zhong", "中种重众钟终忠"},
    {"zhou", "周州洲粥轴宙皱"},
    {"zhu", "主住注助猪竹筑"},
    {"zhua", "抓爪"},
    {"zhuai", "拽"},
    {"zhuan", "转专赚砖撰"},
    {"zhuang", "装状壮庄撞妆"},
    {"zhui", "追坠缀锥赘"},
    {"zhun", "准谆"},
    {"zhuo", "桌捉着灼浊啄"},
    {"zi", "自子字资紫仔姿"},
    {"zui", "最罪嘴醉"},
    {"zuo", "做左作坐座昨"},
    {"zong", "总宗纵踪棕粽"},
    {"zou", "走奏揍邹"},
    {NULL, NULL}};
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t *lv_ime_pinyin_create(lv_obj_t *parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Setter functions
 *====================*/

void lv_ime_pinyin_set_keyboard(lv_obj_t *obj, lv_obj_t *kb)
{
    if (kb)
    {
        LV_ASSERT_OBJ(kb, &lv_keyboard_class);
    }

    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->kb = kb;
    lv_obj_set_parent(obj, lv_obj_get_parent(kb));
    lv_obj_set_parent(pinyin_ime->cand_panel, lv_obj_get_parent(kb));
    lv_obj_add_event_cb(pinyin_ime->kb, lv_ime_pinyin_kb_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_align_to(pinyin_ime->cand_panel, pinyin_ime->kb, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

void lv_ime_pinyin_set_dict(lv_obj_t *obj, lv_pinyin_dict_t *dict)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    init_pinyin_dict(obj, dict);
}

void lv_ime_pinyin_set_mode(lv_obj_t *obj, lv_ime_pinyin_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    LV_ASSERT_OBJ(pinyin_ime->kb, &lv_keyboard_class);

    pinyin_ime->mode = mode;

#if LV_IME_PINYIN_USE_K9_MODE
    if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K9)
    {
        pinyin_k9_init_data(obj);
        lv_keyboard_set_map(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1, (const char **)lv_btnm_def_pinyin_k9_map,
                            default_kb_ctrl_k9_map);
        lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_USER_1);
    }
#endif
}

/*=====================
 * Getter functions
 *====================*/

lv_obj_t *lv_ime_pinyin_get_kb(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->kb;
}

lv_obj_t *lv_ime_pinyin_get_cand_panel(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->cand_panel;
}

const lv_pinyin_dict_t *lv_ime_pinyin_get_dict(lv_obj_t *obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    return pinyin_ime->dict;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_ime_pinyin_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for (btnm_i = 0; btnm_i < (LV_IME_PINYIN_CAND_TEXT_NUM + 3); btnm_i++)
    {
        if (btnm_i == 0)
        {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "<";
        }
        else if (btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 1))
        {
            lv_btnm_def_pinyin_sel_map[btnm_i] = ">";
        }
        else if (btnm_i == (LV_IME_PINYIN_CAND_TEXT_NUM + 2))
        {
            lv_btnm_def_pinyin_sel_map[btnm_i] = "";
        }
        else
        {
            lv_pinyin_cand_str[py_str_i][0] = ' ';
            lv_btnm_def_pinyin_sel_map[btnm_i] = lv_pinyin_cand_str[py_str_i];
            py_str_i++;
        }
    }

    pinyin_ime->mode = LV_IME_PINYIN_MODE_K26;
    pinyin_ime->py_page = 0;
    pinyin_ime->ta_count = 0;
    pinyin_ime->cand_num = 0;
    lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
    lv_memzero(pinyin_ime->py_num, sizeof(pinyin_ime->py_num));
    lv_memzero(pinyin_ime->py_pos, sizeof(pinyin_ime->py_pos));

    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

#if LV_IME_PINYIN_USE_DEFAULT_DICT
    init_pinyin_dict(obj, lv_ime_pinyin_def_dict);
#endif

    /* Init pinyin_ime->cand_panel */
    pinyin_ime->cand_panel = lv_buttonmatrix_create(lv_obj_get_parent(obj));
    lv_buttonmatrix_set_map(pinyin_ime->cand_panel, (const char **)lv_btnm_def_pinyin_sel_map);
    lv_obj_set_size(pinyin_ime->cand_panel, LV_PCT(100), LV_PCT(5));
    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);

    lv_buttonmatrix_set_one_checked(pinyin_ime->cand_panel, true);
    lv_obj_remove_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    /* Set cand_panel style*/
    // Default style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_all(pinyin_ime->cand_panel, 8, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_pad_gap(pinyin_ime->cand_panel, 0, 0);
    lv_obj_set_style_base_dir(pinyin_ime->cand_panel, LV_BASE_DIR_LTR, 0);

    // LV_PART_ITEMS style
    lv_obj_set_style_radius(pinyin_ime->cand_panel, 12, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(pinyin_ime->cand_panel, LV_OPA_0, LV_PART_ITEMS);

    // LV_PART_ITEMS | LV_STATE_PRESSED style
    lv_obj_set_style_bg_opa(pinyin_ime->cand_panel, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(pinyin_ime->cand_panel, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);

    /* event handler */
    lv_obj_add_event_cb(pinyin_ime->cand_panel, lv_ime_pinyin_cand_panel_event, LV_EVENT_VALUE_CHANGED, obj);
    lv_obj_add_event_cb(obj, lv_ime_pinyin_style_change_event, LV_EVENT_STYLE_CHANGED, NULL);

#if LV_IME_PINYIN_USE_K9_MODE
    pinyin_ime->k9_input_str_len = 0;
    pinyin_ime->k9_py_ll_pos = 0;
    pinyin_ime->k9_legal_py_count = 0;
    lv_memzero(pinyin_ime->k9_input_str, LV_IME_PINYIN_K9_MAX_INPUT);

    pinyin_k9_init_data(obj);

    _lv_ll_init(&(pinyin_ime->k9_legal_py_ll), sizeof(ime_pinyin_k9_py_str_t));
#endif
}

static void lv_ime_pinyin_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    LV_UNUSED(class_p);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    if (lv_obj_is_valid(pinyin_ime->kb))
        lv_obj_delete(pinyin_ime->kb);

    if (lv_obj_is_valid(pinyin_ime->cand_panel))
        lv_obj_delete(pinyin_ime->cand_panel);
}

static void lv_ime_pinyin_kb_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_current_target(e);
    lv_obj_t *obj = lv_event_get_user_data(e);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    static const char *k9_py_map[8] = {"abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
#endif

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t btn_id = lv_buttonmatrix_get_selected_button(kb);
        if (btn_id == LV_BUTTONMATRIX_BUTTON_NONE)
            return;

        const char *txt = lv_buttonmatrix_get_button_text(kb, lv_buttonmatrix_get_selected_button(kb));
        if (txt == NULL)
            return;

        lv_obj_t *ta = lv_keyboard_get_textarea(pinyin_ime->kb);

#if LV_IME_PINYIN_USE_K9_MODE
        if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K9)
        {

            uint16_t tmp_button_str_len = lv_strlen(pinyin_ime->input_char);
            if ((btn_id >= 16) && (tmp_button_str_len > 0) && (btn_id < (16 + LV_IME_PINYIN_K9_CAND_TEXT_NUM)))
            {
                lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));
                strcat(pinyin_ime->input_char, txt);
                pinyin_input_proc(obj);

                for (int index = 0; index < (pinyin_ime->ta_count + tmp_button_str_len); index++)
                {
                    lv_textarea_delete_char(ta);
                }

                pinyin_ime->ta_count = tmp_button_str_len;
                pinyin_ime->k9_input_str_len = tmp_button_str_len;
                lv_textarea_add_text(ta, pinyin_ime->input_char);

                return;
            }
        }
#endif

        if (lv_strcmp(txt, "Enter") == 0 || lv_strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
        {
            pinyin_ime_clear_data(obj);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if (lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
        {
            // del input char
            if (pinyin_ime->ta_count > 0)
            {
                if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
                    pinyin_ime->input_char[pinyin_ime->ta_count - 1] = '\0';
#if LV_IME_PINYIN_USE_K9_MODE
                else
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count - 1] = '\0';
#endif

                pinyin_ime->ta_count--;
                if (pinyin_ime->ta_count <= 0)
                {
                    pinyin_ime_clear_data(obj);
                    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
                }
                else if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
                {
                    pinyin_input_proc(obj);
                }
#if LV_IME_PINYIN_USE_K9_MODE
                else if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K9)
                {
                    pinyin_ime->k9_input_str_len = lv_strlen(pinyin_ime->input_char) - 1;
                    pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
                    pinyin_k9_fill_cand(obj);
                    pinyin_input_proc(obj);
                    pinyin_ime->ta_count--;
                }
#endif
            }
        }
        else if ((lv_strcmp(txt, "ABC") == 0) || (lv_strcmp(txt, "abc") == 0) || (lv_strcmp(txt, "1#") == 0) ||
                 (lv_strcmp(txt, LV_SYMBOL_OK) == 0))
        {
            pinyin_ime_clear_data(obj);
            return;
        }
        else if (lv_strcmp(txt, "123") == 0)
        {
            for (uint16_t i = 0; i < lv_strlen(txt); i++)
                lv_textarea_delete_char(ta);

            pinyin_ime_clear_data(obj);
            lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
            lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9_NUMBER);
            lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
            lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
        }
        else if (lv_strcmp(txt, LV_SYMBOL_KEYBOARD) == 0)
        {
            if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K26)
            {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9);
            }
            else if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K9)
            {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K26);
                lv_keyboard_set_mode(pinyin_ime->kb, LV_KEYBOARD_MODE_TEXT_LOWER);
            }
            else if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K9_NUMBER)
            {
                lv_ime_pinyin_set_mode(obj, LV_IME_PINYIN_MODE_K9);
            }
            pinyin_ime_clear_data(obj);
        }
        else if ((pinyin_ime->mode == LV_IME_PINYIN_MODE_K26) && ((txt[0] >= 'a' && txt[0] <= 'z') || (txt[0] >= 'A' &&
                                                                                                       txt[0] <= 'Z')))
        {
            strcat(pinyin_ime->input_char, txt);
            pinyin_input_proc(obj);
            pinyin_ime->ta_count++;
        }
#if LV_IME_PINYIN_USE_K9_MODE
        else if ((pinyin_ime->mode == LV_IME_PINYIN_MODE_K9) && (txt[0] >= 'a' && txt[0] <= 'z'))
        {
            for (uint16_t i = 0; i < 8; i++)
            {
                if ((lv_strcmp(txt, k9_py_map[i]) == 0) || (lv_strcmp(txt, "abc ") == 0))
                {
                    if (lv_strcmp(txt, "abc ") == 0)
                        pinyin_ime->k9_input_str_len += lv_strlen(k9_py_map[i]) + 1;
                    else
                        pinyin_ime->k9_input_str_len += lv_strlen(k9_py_map[i]);
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count] = 50 + i;
                    pinyin_ime->k9_input_str[pinyin_ime->ta_count + 1] = '\0';

                    break;
                }
            }
            pinyin_k9_get_legal_py(obj, pinyin_ime->k9_input_str, k9_py_map);
            pinyin_k9_fill_cand(obj);
            pinyin_input_proc(obj);
        }
        else if (lv_strcmp(txt, LV_SYMBOL_LEFT) == 0)
        {
            pinyin_k9_cand_page_proc(obj, 0);
        }
        else if (lv_strcmp(txt, LV_SYMBOL_RIGHT) == 0)
        {
            pinyin_k9_cand_page_proc(obj, 1);
        }
#endif
    }
}

static void lv_ime_pinyin_cand_panel_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *cand_panel = lv_event_get_current_target(e);
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_user_data(e);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        lv_obj_t *ta = lv_keyboard_get_textarea(pinyin_ime->kb);
        if (ta == NULL)
            return;

        uint32_t id = lv_buttonmatrix_get_selected_button(cand_panel);
        if (id == LV_BUTTONMATRIX_BUTTON_NONE)
        {
            return;
        }
        else if (id == 0)
        {
            pinyin_page_proc(obj, 0);
            return;
        }
        else if (id == (LV_IME_PINYIN_CAND_TEXT_NUM + 1))
        {
            pinyin_page_proc(obj, 1);
            return;
        }

        const char *txt = lv_buttonmatrix_get_button_text(cand_panel, id);
        uint16_t index = 0;
        for (index = 0; index < pinyin_ime->ta_count; index++)
            lv_textarea_delete_char(ta);

        lv_textarea_add_text(ta, txt);

        pinyin_ime_clear_data(obj);
    }
}

static void pinyin_input_proc(lv_obj_t *obj)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    pinyin_ime->cand_str = pinyin_search_matching(obj, pinyin_ime->input_char, &pinyin_ime->cand_num);
    if (pinyin_ime->cand_str == NULL)
    {
        return;
    }

    pinyin_ime->py_page = 0;

    for (uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++)
    {
        lv_memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    for (uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++)
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[i * 3 + j];
        }
    }

    lv_obj_remove_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}

static void pinyin_page_proc(lv_obj_t *obj, uint16_t dir)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;
    uint16_t page_num = pinyin_ime->cand_num / LV_IME_PINYIN_CAND_TEXT_NUM;
    uint16_t remainder = pinyin_ime->cand_num % LV_IME_PINYIN_CAND_TEXT_NUM;

    if (!pinyin_ime->cand_str)
        return;

    if (dir == 0)
    {
        if (pinyin_ime->py_page)
        {
            pinyin_ime->py_page--;
        }
    }
    else
    {
        if (remainder == 0)
        {
            page_num -= 1;
        }
        if (pinyin_ime->py_page < page_num)
        {
            pinyin_ime->py_page++;
        }
        else
            return;
    }

    for (uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++)
    {
        lv_memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }

    // fill buf
    uint16_t offset = pinyin_ime->py_page * (3 * LV_IME_PINYIN_CAND_TEXT_NUM);
    for (uint8_t i = 0; (i < pinyin_ime->cand_num && i < LV_IME_PINYIN_CAND_TEXT_NUM); i++)
    {
        if ((remainder > 0) && (pinyin_ime->py_page == page_num))
        {
            if (i > remainder)
                break;
        }
        for (uint8_t j = 0; j < 3; j++)
        {
            lv_pinyin_cand_str[i][j] = pinyin_ime->cand_str[offset + (i * 3) + j];
        }
    }
}

static void lv_ime_pinyin_style_change_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target(e);

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    if (code == LV_EVENT_STYLE_CHANGED)
    {
        const lv_font_t *font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_obj_set_style_text_font(pinyin_ime->cand_panel, font, 0);
    }
}

static void init_pinyin_dict(lv_obj_t *obj, const lv_pinyin_dict_t *dict)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    char headletter = 'a';
    uint16_t offset_sum = 0;
    uint16_t offset_count = 0;
    uint16_t letter_calc = 0;

    pinyin_ime->dict = dict;

    for (uint16_t i = 0;; i++)
    {
        if ((NULL == (dict[i].py)) || (NULL == (dict[i].py_mb)))
        {
            headletter = dict[i - 1].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc] = offset_count;
            break;
        }

        if (headletter == (dict[i].py[0]))
        {
            offset_count++;
        }
        else
        {
            headletter = dict[i].py[0];
            letter_calc = headletter - 'a';
            pinyin_ime->py_num[letter_calc - 1] = offset_count;
            offset_sum += offset_count;
            pinyin_ime->py_pos[letter_calc] = offset_sum;

            offset_count = 1;
        }
    }
}

static char *pinyin_search_matching(lv_obj_t *obj, char *py_str, uint16_t *cand_num)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    const lv_pinyin_dict_t *cpHZ;
    uint8_t index, len = 0, offset;
    volatile uint8_t count = 0;

    if (*py_str == '\0')
        return NULL;
    if (*py_str == 'i')
        return NULL;
    if (*py_str == 'u')
        return NULL;
    if (*py_str == 'v')
        return NULL;
    if (*py_str == ' ')
        return NULL;

    offset = py_str[0] - 'a';
    len = lv_strlen(py_str);

    cpHZ = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while (count--)
    {
        for (index = 0; index < len; index++)
        {
            if (*(py_str + index) != *((cpHZ->py) + index))
            {
                break;
            }
        }

        // perfect match
        if (len == 1 || index == len)
        {
            // The Chinese character in UTF-8 encoding format is 3 bytes
            *cand_num = lv_strlen((const char *)(cpHZ->py_mb)) / 3;
            return (char *)(cpHZ->py_mb);
        }
        cpHZ++;
    }
    return NULL;
}

static void pinyin_ime_clear_data(lv_obj_t *obj)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

#if LV_IME_PINYIN_USE_K9_MODE
    if (pinyin_ime->mode == LV_IME_PINYIN_MODE_K9)
    {
        pinyin_ime->k9_input_str_len = 0;
        pinyin_ime->k9_py_ll_pos = 0;
        pinyin_ime->k9_legal_py_count = 0;
        lv_memzero(pinyin_ime->k9_input_str, LV_IME_PINYIN_K9_MAX_INPUT);
        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        for (uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++)
        {
            lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
        }
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT "\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        lv_buttonmatrix_set_map(pinyin_ime->kb, (const char **)lv_btnm_def_pinyin_k9_map);
    }
#endif

    pinyin_ime->ta_count = 0;
    for (uint8_t i = 0; i < LV_IME_PINYIN_CAND_TEXT_NUM; i++)
    {
        lv_memset(lv_pinyin_cand_str[i], 0x00, sizeof(lv_pinyin_cand_str[i]));
        lv_pinyin_cand_str[i][0] = ' ';
    }
    lv_memzero(pinyin_ime->input_char, sizeof(pinyin_ime->input_char));

    lv_obj_add_flag(pinyin_ime->cand_panel, LV_OBJ_FLAG_HIDDEN);
}

#if LV_IME_PINYIN_USE_K9_MODE
static void pinyin_k9_init_data(lv_obj_t *obj)
{
    LV_UNUSED(obj);

    uint16_t py_str_i = 0;
    uint16_t btnm_i = 0;
    for (btnm_i = 19; btnm_i < (LV_IME_PINYIN_K9_CAND_TEXT_NUM + 21); btnm_i++)
    {
        if (py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM)
        {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], LV_SYMBOL_RIGHT "\0");
        }
        else if (py_str_i == LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1)
        {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], "\0");
        }
        else
        {
            lv_strcpy(lv_pinyin_k9_cand_str[py_str_i], " \0");
        }

        lv_btnm_def_pinyin_k9_map[btnm_i] = lv_pinyin_k9_cand_str[py_str_i];
        py_str_i++;
    }

    default_kb_ctrl_k9_map[0] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[1] = LV_BUTTONMATRIX_CTRL_NO_REPEAT | LV_BUTTONMATRIX_CTRL_CLICK_TRIG | 1;
    default_kb_ctrl_k9_map[4] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[5] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[9] = LV_KEYBOARD_CTRL_BUTTON_FLAGS | 1;
    default_kb_ctrl_k9_map[10] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[14] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[15] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
    default_kb_ctrl_k9_map[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 16] = LV_BUTTONMATRIX_CTRL_CHECKED | 1;
}

static void pinyin_k9_get_legal_py(lv_obj_t *obj, char *k9_input, const char *py9_map[])
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    uint16_t len = lv_strlen(k9_input);

    if ((len == 0) || (len >= LV_IME_PINYIN_K9_MAX_INPUT))
    {
        return;
    }

    char py_comp[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int mark[LV_IME_PINYIN_K9_MAX_INPUT] = {0};
    int index = 0;
    int flag = 0;
    uint16_t count = 0;

    uint32_t ll_len = 0;
    ime_pinyin_k9_py_str_t *ll_index = NULL;

    ll_len = _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);
    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);

    while (index != -1)
    {
        if (index == len)
        {
            if (pinyin_k9_is_valid_py(obj, py_comp))
            {
                if ((count >= ll_len) || (ll_len == 0))
                {
                    ll_index = _lv_ll_ins_tail(&pinyin_ime->k9_legal_py_ll);
                    lv_strcpy(ll_index->py_str, py_comp);
                }
                else if ((count < ll_len))
                {
                    lv_strcpy(ll_index->py_str, py_comp);
                    ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index);
                }
                count++;
            }
            index--;
        }
        else
        {
            flag = mark[index];
            if ((size_t)flag < lv_strlen(py9_map[k9_input[index] - '2']))
            {
                py_comp[index] = py9_map[k9_input[index] - '2'][flag];
                mark[index] = mark[index] + 1;
                index++;
            }
            else
            {
                mark[index] = 0;
                index--;
            }
        }
    }

    if (count > 0)
    {
        pinyin_ime->ta_count++;
        pinyin_ime->k9_legal_py_count = count;
    }
}

/*true: visible; false: not visible*/
static bool pinyin_k9_is_valid_py(lv_obj_t *obj, char *py_str)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    const lv_pinyin_dict_t *cpHZ = NULL;
    uint8_t index = 0, len = 0, offset = 0;
    volatile uint8_t count = 0;

    if (*py_str == '\0')
        return false;
    if (*py_str == 'i')
        return false;
    if (*py_str == 'u')
        return false;
    if (*py_str == 'v')
        return false;

    offset = py_str[0] - 'a';
    len = lv_strlen(py_str);

    cpHZ = &pinyin_ime->dict[pinyin_ime->py_pos[offset]];
    count = pinyin_ime->py_num[offset];

    while (count--)
    {
        for (index = 0; index < len; index++)
        {
            if (*(py_str + index) != *((cpHZ->py) + index))
            {
                break;
            }
        }

        // perfect match
        if (len == 1 || index == len)
        {
            return true;
        }
        cpHZ++;
    }
    return false;
}

static void pinyin_k9_fill_cand(lv_obj_t *obj)
{
    uint16_t index = 0, tmp_len = 0;
    ime_pinyin_k9_py_str_t *ll_index = NULL;

    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    tmp_len = pinyin_ime->k9_legal_py_count;

    if (tmp_len != cand_len)
    {
        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT "\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");
        cand_len = tmp_len;
    }

    ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
    lv_strcpy(pinyin_ime->input_char, ll_index->py_str);

    for (uint8_t i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++)
    {
        lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
    }

    while (ll_index)
    {
        if (index >= LV_IME_PINYIN_K9_CAND_TEXT_NUM)
            break;

        if (index < pinyin_ime->k9_legal_py_count)
        {
            lv_strcpy(lv_pinyin_k9_cand_str[index], ll_index->py_str);
        }

        ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
        index++;
    }
    pinyin_ime->k9_py_ll_pos = index;

    lv_obj_t *ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    for (index = 0; index < pinyin_ime->k9_input_str_len; index++)
    {
        lv_textarea_delete_char(ta);
    }
    pinyin_ime->k9_input_str_len = lv_strlen(pinyin_ime->input_char);
    lv_textarea_add_text(ta, pinyin_ime->input_char);
}

static void pinyin_k9_cand_page_proc(lv_obj_t *obj, uint16_t dir)
{
    lv_ime_pinyin_t *pinyin_ime = (lv_ime_pinyin_t *)obj;

    lv_obj_t *ta = lv_keyboard_get_textarea(pinyin_ime->kb);
    uint16_t ll_len = _lv_ll_get_len(&pinyin_ime->k9_legal_py_ll);

    if ((ll_len > LV_IME_PINYIN_K9_CAND_TEXT_NUM) && (pinyin_ime->k9_legal_py_count > LV_IME_PINYIN_K9_CAND_TEXT_NUM))
    {
        ime_pinyin_k9_py_str_t *ll_index = NULL;
        int count = 0;

        ll_index = _lv_ll_get_head(&pinyin_ime->k9_legal_py_ll);
        while (ll_index)
        {
            if (count >= pinyin_ime->k9_py_ll_pos)
                break;

            ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
            count++;
        }

        if ((NULL == ll_index) && (dir == 1))
            return;

        lv_memzero(lv_pinyin_k9_cand_str, sizeof(lv_pinyin_k9_cand_str));
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM], LV_SYMBOL_RIGHT "\0");
        lv_strcpy(lv_pinyin_k9_cand_str[LV_IME_PINYIN_K9_CAND_TEXT_NUM + 1], "\0");

        // next page
        if (dir == 1)
        {
            for (uint8_t i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++)
            {
                lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
            }

            count = 0;
            while (ll_index)
            {
                if (count >= (LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1))
                    break;

                lv_strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_next(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the next list*/
                count++;
            }
            pinyin_ime->k9_py_ll_pos += count - 1;
        }
        // previous page
        else
        {
            for (uint8_t i = 0; i < LV_IME_PINYIN_K9_CAND_TEXT_NUM; i++)
            {
                lv_strcpy(lv_pinyin_k9_cand_str[i], " ");
            }
            count = LV_IME_PINYIN_K9_CAND_TEXT_NUM - 1;
            ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index);
            while (ll_index)
            {
                if (count < 0)
                    break;

                lv_strcpy(lv_pinyin_k9_cand_str[count], ll_index->py_str);
                ll_index = _lv_ll_get_prev(&pinyin_ime->k9_legal_py_ll, ll_index); /*Find the previous list*/
                count--;
            }

            if (pinyin_ime->k9_py_ll_pos > LV_IME_PINYIN_K9_CAND_TEXT_NUM)
                pinyin_ime->k9_py_ll_pos -= 1;
        }

        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
    }
}

#endif /*LV_IME_PINYIN_USE_K9_MODE*/

#endif /*LV_USE_IME_PINYIN*/
