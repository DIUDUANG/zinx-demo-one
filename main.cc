#include <cstdio>
#include <iostream>
#include <map>
#include "zinx.h"



//数据处理类echo,继承于AZinxHandler
class echo : public AZinxHandler {
  // 通过 AZinxHandler 继承
  virtual IZinxMsg* InternelHandle(IZinxMsg& _oInput) override {

    GET_REF2DATA(BytesMsg, byte, _oInput);
    Ichannel* pChannel = ZinxKernel::Zinx_GetChannel_ByInfo("stdout_channel");
    if (NULL != pChannel) {
      ZinxKernel::Zinx_SendOut(byte.szData, *pChannel);
    }

    return nullptr;
  }
  virtual AZinxHandler* GetNextHandler(IZinxMsg& _oNextMsg) override {
    return nullptr;
  }
};

echo g_echo;





//退出框架,继承于AZinxHandler
class exitFramework : public AZinxHandler {

  // 通过 AZinxHandler 继承
  virtual IZinxMsg* InternelHandle(IZinxMsg& _oInput) override {

    ZinxKernel::Zinx_Exit();
    return nullptr;
  }

  virtual AZinxHandler* GetNextHandler(IZinxMsg& _oNextMsg) override {
    return nullptr;
  }
};

exitFramework g_exit;






//输出管理类output_mng,继承于AZinxHandler
class output_mng : public AZinxHandler {

  Ichannel* m_channel = NULL;
  // 通过 AZinxHandler 继承
  virtual IZinxMsg* InternelHandle(IZinxMsg& _oInput) override {

    if (NULL == m_channel) {
      Ichannel* pChannel = ZinxKernel::Zinx_GetChannel_ByInfo("stdout_channel");
      m_channel = pChannel;
    }

    GET_REF2DATA(BytesMsg, byte, _oInput);

    if ("close" == byte.szData) {
      ZinxKernel::Zinx_Del_Channel(*m_channel);
    }
    else {
      ZinxKernel::Zinx_Add_Channel(*m_channel);
    }

    return nullptr;
  }

  virtual AZinxHandler* GetNextHandler(IZinxMsg& _oNextMsg) override {
    return nullptr;
  }

};

output_mng g_output_mng;




class Cmd : public AZinxHandler {

  std::map<std::string, AZinxHandler*> m_cmd_map;

  // 通过 AZinxHandler 继承
  virtual IZinxMsg* InternelHandle(IZinxMsg& _oInput) override {
    
    GET_REF2DATA(BytesMsg, byte, _oInput);
    return new BytesMsg(byte);

    return nullptr;
  }


  virtual AZinxHandler* GetNextHandler(IZinxMsg& _oNextMsg) override {

    GET_REF2DATA(BytesMsg, byte, _oNextMsg);

    if (m_cmd_map.end() != m_cmd_map.find(byte.szData)) {
      return m_cmd_map[byte.szData];
    }
    else {
      return &g_echo;
    }

    return nullptr;
  }

 public:
   void add_cmd(std::string _cmd, AZinxHandler* _handler) {
     m_cmd_map[_cmd] = _handler;
  }
};

Cmd g_cmd;



//输入通道类stdin_channel,继承于Ichannel
class stdin_channel : public Ichannel {
  // 通过 Ichannel 继承
  virtual bool Init() override {
    return true;
  }

  virtual bool ReadFd(std::string& _input) override {

    std::cin >> _input;
    return true;
  }

  virtual bool WriteFd(std::string& _output) override {
    return false;
  }

  virtual void Fini() override {
  }

  virtual int GetFd() override {
    return STDIN_FILENO;
  }

  virtual std::string GetChannelInfo() override {
    return "stdin_channel";
  }

  virtual AZinxHandler* GetInputNextStage(BytesMsg& _oInput) override {

    return &g_cmd;
  }
};



//创建标准输出通道
class stdout_channel : public Ichannel {
  // 通过 Ichannel 继承
  virtual bool Init() override {
    return true;
  }

  virtual bool ReadFd(std::string& _input) override {
    return false;
  }

  virtual bool WriteFd(std::string& _output) override {

    std::cout << '[' << _output << ']' << std::endl;
    return true;
  }

  virtual void Fini() override {
  }

  virtual int GetFd() override {
    return STDOUT_FILENO;
  }

  virtual std::string GetChannelInfo() override {
    return "stdout_channel";
  }

  virtual AZinxHandler* GetInputNextStage(BytesMsg& _oInput) override {
    return nullptr;
  }
};





int main(int argc, char** argv) {
  
  //初始化框架
  ZinxKernel::ZinxKernelInit();

  //创建通道类
  stdin_channel* pStdin_channel = new stdin_channel();
  stdout_channel* pStdout_channel = new stdout_channel();

  //将通道加入
  ZinxKernel::Zinx_Add_Channel(*pStdout_channel);
  ZinxKernel::Zinx_Add_Channel(*pStdin_channel);

  //想输入控制类绑定命令
  g_cmd.add_cmd("exit", &g_exit);
  g_cmd.add_cmd("close", &g_output_mng);
  g_cmd.add_cmd("open", &g_output_mng);

  //运行框架
  ZinxKernel::Zinx_Run();

  //删除通道
  ZinxKernel::Zinx_Del_Channel(*pStdout_channel);
  ZinxKernel::Zinx_Del_Channel(*pStdin_channel);

  //删除堆区通道
  delete pStdin_channel;
  delete pStdout_channel;
  

  //结束框架
  ZinxKernel::ZinxKernelFini();

  return 0;
}
