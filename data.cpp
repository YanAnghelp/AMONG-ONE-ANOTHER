// 游戏存档&保存数据处理接口，欢迎进行优化或提供建议
// 
// Powred By Yan

#include <mutex>
import logger;

struct {
	int TESTdata;
} UData;

// 安全操作接口
// 
// 我为结构体里的每一个变量都提供了一个安全操作接口
// 请勿直接调用结构体！！请勿直接调用结构体！！请勿直接调用结构体！！
// 操作接口命名规律：读操作接口为 UDataW_变量名；写操作接口：UDataR_变量名
// 由于结构体只有一个互斥锁，读写速度很可能会变慢，未来考虑多互斥锁。
// 但是非原子性质变量有可能发生冲突，此问题亟待解决
// 如果有更好的方法欢迎讨论！
namespace {
	std::mutex UDataMutex;

	// TESTdata
	int UDataR_TESTdata() {
		std::lock_guard<std::mutex> guard(UDataMutex);
		return UData.TESTdata;
	}
	void UDataW_TESTdata(int data) {
		std::lock_guard<std::mutex> guard(UDataMutex);
		UData.TESTdata = data;
	}
}