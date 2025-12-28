一,项目总体介绍

这是一款类部落冲突游戏,实现了多类兵种,多类建筑,多张地图,游戏音乐,游戏音效,自动战斗系统.

该项目做了以下加分项的工作:

1.成功运行于安卓系统

2.全程采用github托管,便于控制版本,提交commit记录清晰.工作按阶段开展,组员分配不同的功能进行实现

3.代码质量：

无内存泄漏：项目广泛使用了 Cocos2d-x 的内存管理机制（如 autorelease、Ref 计数）以及 C++ 标准库容器（std::vector, std::map），有效避免了手动管理内存带来的泄漏风险。

合理异常处理：虽然没有大量使用 try-catch 块，但在关键逻辑处（如文件加载、对象创建、空指针检查）都有相应的判断和处理，例如 Building::create 中的空指针检查和 MainMode::tryBuyBuilding 中的资源检查。

4.开发特性：

C++11/14 特性使用丰富：代码中大量使用了 auto 类型推导、Lambda 表达式（特别是在回调函数中）、std::function、std::bind、std::chrono 等现代 C++ 特性，代码简洁且现代化。

优雅的架构设计：采用了典型的 MVC 分层思想，将数据管理（GameManager）、逻辑处理（BattleManager）、视图展示（MainModeScene, BattleScene）和实体对象（GameEntity, Building, Troop）分离，结构清晰，职责分明。

目录结构清晰：类文件按功能模块划分，头文件与源文件一一对应，便于维护和查找。

界面与体验：

界面精美：采用高质量的游戏素材,ui布局合理,大本营超大地图(可随鼠标拖动移动)

流畅动画：使用了 Cocos2d-x 的动作系统（Action、Sequence、Spawn、Animate）来实现建筑升级、资源收集、士兵移动攻击等动画效果，增强了游戏的视觉表现力。

游戏不卡顿不崩溃：通过合理的资源管理和逻辑更新（update 循环），以及对潜在崩溃点（如迭代器失效）的修复（如 Building::onDeath 中的延迟移除），保证了游戏的稳定性。

二,项目简略结构如下:

classDiagram

    核心管理层
    
    -class GameManager 
    
    -class BattleManager 
    
    场景层
    
    -class MainModeScene     //主场景 大本营所在地
    
    -class BattleScene       //战斗场景
    
    -class OpenScene        //游戏开始场景
    
    -class LevelMapScene    //关卡选择场景
    
    实体层(基类)
    
    -class GameEntity       //所有具有属性的对象所在类
    
    -class Building         //Building基类
    
    -class Troop            //Troop基类

    实体层(继承类)

    class Buildings        

    -class Archertower       //弓箭塔

    -class Barracks          //军营

    -class Cannon            //加农炮

    -class ElixirCollector   //圣水收集器

    -class ElixirStorage    //圣水存储器

    -class GoldMine          //金矿

    -class GoldStorage       //金库

    -class TownHall          //大本营

    -class Trap              //陷阱

    -class Wall              //城墙

    class Trops:

    -class Barbarian         //野蛮人

    -class Archer            //弓箭手

    -class Bomberman        //炸弹人

    -class dragon            //飞龙

    -class giant            //巨人
    
    UI层
    
    -class BarracksUI
    
    -class GameUI
    
    -class MenuBuilder
    
    -class RecruitUI
    
    -class UpgradeTroopUI
    
    关系
    
    GameEntity <|-- Building : 继承 (Inheritance)
    
    GameEntity <|-- Troop : 继承 (Inheritance)
    
    MainModeScene --> GameManager : 调用 (Uses)
    
    MainModeScene --> Building : 管理 (Manages)
    
    Building --> GameManager : 读取配置/存档 (Uses)
    
    GameManager --> Building : 调度升级 (Schedules)
    
功能介绍:

1. 👑 核心指挥中心 (Managers)

•	GameManager (大管家):

•	人设: 城堡里最忙碌的管家，随身带着账本。

•	职责:

•	管钱: 记录金币、圣水有多少 (m_gold, m_elixir)。

•	管人: 知道你训练了多少个野蛮人、弓箭手 (m_troopCounts)。

•	记性好: 负责把你的建筑等级、位置写在小本本上 (save/load)，切回场景时还在。

•	监工: 盯着建筑升级的倒计时，就算你下线了，他也在掐表 (scheduleBuildingUpgrade),在你远征时也会完成建筑升级和加速功能。

•	BattleManager (战争将军):

•	人设: 铁血将军，只在打仗时出现。

•	职责:

•	战场指挥: 知道战场上每一个士兵和建筑的位置。

•	索敌: 告诉防御塔“打那个最近的敌人！”(findClosestTroop)。

•	裁决: 计算伤害，判定谁死了，谁赢了。

3. 🎭 场景舞台 (Scenes)

•	MainModeScene (家园/主城):

•	描述: 你的大本营，一片祥和。

•	活动: 你在这里种田、造房子、收菜、逛商店。这里没有硝烟，只有金币入袋的声音。

•	BattleScene (战场):

•	描述: 硝烟弥漫的前线。

•	活动: 你把训练好的兵投放到这里，看着他们自动去拆对面的家。

•	OpenScene(游戏初始界面)

•	描述:一切的开始

•	活动:点击开始按钮,开始发展你的部落!

•	LevelMapScene(关卡选择界面)

•	描述:选择你的对手,从第一关开始呢,还是直接挑战第三关?

•	活动:点击关卡对应图标,进入关卡,来一场部落间的战斗!

5. 🏠 居民与建筑 (Entities)

•	GameEntity (生物基类):

•	描述: 所有“活物”的祖先。它定义了大家都有血条 (HP)，都会死 (onDeath)。

•	Building (建筑):

•	描述: 不会动的单位。

•	分工:

•	矿机: 默默吐金币 (GoldMine)。

•	防御塔: 看到敌人就突突突 (Cannon, ArcherTower)。

•	兵营: 负责把圣水变成士兵 (Barracks)。

•	大本营: 你的命根子，爆了就输了。

•	Troop (士兵):

•	描述: 会动的单位，只会干两件事：移动和攻击。

•	性格: 有的血厚（巨人），有的手长（弓箭手），有的会飞（飞龙）。

7. 🖼️ 告示板 (UI)

•	GameUI: 屏幕上方的横条，告诉你现在多有钱。

•	BarracksUI: 征兵处的窗口，点一下训练一个兵,还可以升级你的军营,使你的士兵数量更多更丰富.

•	ShopUI: 建筑商店，只要有钱，想造啥造啥。

---
🚀 它们是如何协作的？
1.	造兵流程:

•	你在 BarracksUI 点了“训练野蛮人”。

•	GameManager (大管家) 检查圣水够不够，够就扣钱，并在小本本上记一笔“野蛮人+1”。

3.	战斗流程:

•	你进入 BattleScene。

•	BattleManager (将军) 从 GameManager 那里拿到兵力数据。

•	你点击地面，Troop (士兵) 生成。

•	士兵问将军：“最近的敌人在哪？”

•	将军指路，士兵冲过去攻击 Building。

•	Building (防御塔) 也问将军：“谁进我射程了？”

•	将军指路，防御塔开火。

•	经过激烈战斗,是你死还是我活?如果对方大本营被摧毁,那么宣判你的胜利,倘若士兵无人生还,军营也没有多余士兵,你只好接受失败,重头来过.

项目做到了以下基本要求:

代码中多处用到了STL容器:

•	std::vector:

•	用法: 用于存储动态增长的数据列表。

•	代码: std::vector<BuildingData> m_homeBuildings; (存储已建造的建筑存档)，std::vector<PendingBuildingUpgrade> m_pendingUpgrades; (存储正在升级的任务队列)。

•	std::map:

•	用法: 用于键值对映射，快速查找配置或状态。

•	代码: std::map<TroopType, int> m_troopCounts; (存储兵种数量)，const std::map<int, BarrackUpgradeConfig> BARRACK_UPGRADE_CONFIGS (兵营升级配置表)。

迭代器:

•	用法: 在容器中查找特定元素或进行遍历删除。

•	代码:如利用迭代器遍历vector,恢复主场景的建筑,如利用迭代器查找特定id的正在施工的建筑

类与多态 (Classes & Polymorphism)

在该项目中，类与多态的使用主要体现在游戏实体的设计上，特别是建筑（Building）和部队（Troop）的继承体系中。
1. 建筑系统 (Building 继承体系)
•	基类 Building:

•	继承自 GameEntity（游戏实体基类）。

•	定义了所有建筑共有的属性和行为，如生命值、等级、升级逻辑、建造动画、资源生产加速等。

•	提供了虚函数接口供子类重写，实现多态行为。

•	工厂模式: Building::create(BuildingType type) 是一个静态工厂方法，根据传入的 BuildingType 枚举值，实例化并返回具体的子类对象（如 GoldMine、Cannon 等）。

•	多态接口:

•	initBuildingProperties(): 纯虚函数（在基类中声明为 = 0)，用于初始化特定建筑的属性（如纹理、血条位置、特定数值）。

•	updateLogic(float dt): 基类提供了通用的加速逻辑，子类（如 GoldMine）重写此函数来实现特定的逻辑（如资源生产、攻击冷却）。

•	onUpgradeFinished(): 基类处理通用的升级效果（等级提升、视觉更新、播放音效），子类可以通过重写或在基类逻辑基础上扩展来实现特定升级效果（如 GoldStorage 增加存储上限）。

•	collectResource(): 基类默认返回 0，资源类建筑（GoldMine, ElixirCollector）重写此函数以返回当前产出的资源量。

•	具体子类:

•	TownHall (大本营): 核心建筑。

•	GoldMine / ElixirCollector (资源生产): 重写了 updateLogic 来处理资源随时间的产出。

•	GoldStorage / ElixirStorage (资源存储): 影响全局资源上限。

•	Cannon / ArcherTower (防御塔): 拥有攻击逻辑。

•	Barracks (军营): 拥有训练部队的逻辑和特有的等级系统 (barrackLevel)。

•	Wall (围墙): 纯防御结构。

•	Trap (陷阱): 特殊防御单位。

3. 部队系统 (Troop 继承体系)

•	基类 Troop:

•	继承自 GameEntity。

•	定义了移动、攻击、寻路等通用行为。

•	具体子类 :

•	Barbarian (野蛮人)

•	Archer (弓箭手)

•	Giant (巨人)

•	Bomberman (炸弹人)

•	Dragon (龙)

•	这些子类会重写基类的攻击方式（近战/远程）、偏好目标（如巨人优先攻击防御建筑）、属性数值等。

5. 游戏管理器 (GameManager)
   
•	GameManager 类本身不体现多态，但它管理着多态对象的集合。

•	m_homeBuildings 存储了 BuildingData，虽然这是数据结构，但在场景加载时（如 MainModeScene），会根据这些数据使用 Building::create 工厂方法恢复出具体的多态 Building 对象。

•	completeBuildingUpgrade 中使用 BattleManager::getInstance()->findBuildingAtPosition 查找到通用的 Building* 指针，然后调用虚函数 onUpgradeFinished()，这是典型的多态应用——管理器不需要知道具体是哪种建筑，只需调用统一接口即可触发正确的升级行为。。

三,玩法介绍:

🏰 部落征服者：战略与荣耀
欢迎来到《部落征服者》！这是一款融合了策略经营与热血战斗的即时战略游戏，由Cocos2d-x引擎精心打造。在这里，你将扮演一位部落首领，从一片荒芜之地开始，一步步建设你的家园，训练强大的军队，并踏上远征之路，征服敌人，夺取资源与荣耀！

✨ 游戏特色
双模式体验：自由切换 经营模式 🏗️ 与 进攻模式 ⚔️，体验建造与征服的双重乐趣！(没有设置防御模式因此大本营无需建造防御设施)

丰富建筑系统：8种功能各异的建筑，每种都可升级强化，打造独一无二的部落城池。

多样兵种搭配：5种特色士兵，包含近战/远程,地面/飞行单位，策略组合决胜千里！

渐进式远征挑战：3道PVE关卡，难度逐步提升，挑战你的战术与运营能力！

资源管理：平衡金币与圣水的采集、储存与消耗，体验真实的首领决策！

🏗️ 经营模式：建设你的部落
在经营模式下，你可以安心规划领地、升级建筑、训练士兵，为远征做好万全准备！

进入游戏左上角有音乐设置,控制背景音乐开关

右上角显示金币和圣水数目以及存储上限

右下角四个按钮有不同的功能:Accelerate键负责实现加速逻辑;Troop键查看士兵数目;Shop控制商店面板;Attack发起进攻,如果没有训练士兵,不允许发起Attack

🏠 建筑一览
shop中能购买以下建筑(陷阱不在此列,但第三关设置有陷阱)
建筑	功能说明	升级效果
大本营 🏛️	部落核心，开局自带，限制其他建筑等级	提升所有建筑等级上限
弓箭塔 🏹	防御设施，自动攻击范围内敌人	提升攻击伤害
金矿 ⛏️	采集金币，资源来源之一	提升采集速率
金币储存器 🪙	存放金币，管理金币上限	提升金币存储上限
圣水泉 💧	采集圣水，珍贵资源来源	提升采集速率
圣水储存器 🫙	存放圣水，管理圣水上限	提升圣水存储上限
军营 ⚔️	训练士兵，招募各类兵种	解锁新兵种、提升士兵上限
城墙 🧱	防御屏障，阻挡敌人进攻	提升生命值（HP）
⚙️ 升级机制
所有建筑升级需要消耗资源与 施工时间 ⏳。

建筑等级 不能超过大本营等级。

升级后建筑 Scale会适当变大，视觉更宏伟！

⚔️ 进攻模式：征服与远征
当你准备就绪，即可进入进攻模式，挑战PVE远征关卡！

🗺️ 远征关卡
共 3道关卡，难度依次递增 🚀。

胜利条件：摧毁敌方大本营 🎯。

奖励：通关后获得大量金币与圣水奖励 💰💧！

🛡️ 敌方设施
弓箭塔：自动攻击你的士兵。

陷阱 🕳️：默认隐形，士兵进入范围后显形并持续造成伤害。

城墙：阻碍前进，需要优先破坏或绕过。

屏障:限制士兵防止范围

🪖 士兵系统
在军营中训练士兵，升级他们，打造无敌军团！

🎖️ 兵种介绍

兵种	类型	特点	升级效果

野蛮人 🪓	近战	基础近战单位，均衡实用	提升HP与伤害

弓箭手 🏹	远程	远程输出，安全攻击	提升HP与伤害

炸弹人 💣	AOE伤害	范围爆炸，对付城墙与集群敌人	提升HP与伤害,优先选中城墙攻破

巨人 🛡️	坦克	高血量，吸引火力	提升HP与伤害

飞龙 🐲	空中单位	空中攻击，陷阱无法选中！	提升HP与伤害

兵种释放,自动战斗,优先选中最近目标(炸弹人除外)

📈 士兵升级

在军营中升级兵种，提升全体该兵种的 HP与伤害。

升级需要消耗资源，但物超所值！

🎮 玩法流程
起步阶段：从大本营开始，建造金矿、圣水泉保障资源。

资源管理：升级储存器提高上限，升级采集建筑加速资源获取。

防御建设：建造弓箭塔与城墙，保卫家园。

军队训练：在军营中训练士兵，升级兵种强化实力。

远征挑战：进入进攻模式，挑战关卡，获取丰厚奖励！

循环发展：用奖励资源继续升级建筑与士兵，挑战更高难度！

⚠️ 注意事项
升级建筑时注意资源与时间的平衡，合理安排施工队列。

兵种搭配至关重要！针对敌方布局选择合适的兵种组合。

飞龙是空中单位，可无视地面陷阱，善加利用！

远征关卡难度逐步提升，建议充分准备后再挑战下一关。

🚀 开始游戏
准备好成为一名伟大的部落首领了吗？现在就开始建设、训练、征服，打造属于你的传奇部落吧！
