// DIVA Object: CompileUnit.
// file : AllObjects.cpp

// DIVA Object: Namespace.
namespace AllObjects {

  // DIVA Object: Alias.
  typedef char character;

  // DIVA Object: Enum.
  enum MyEnum {
    zero,
    one,
    two,
    three
  };

  // DIVA Object: Template, TemplateParameter.
  template <class myTemplateParam>
  class MyDivaObjectTemplate {
  public:
    // DIVA Object: Member, Parameter.
    MyDivaObjectTemplate(myTemplateParam val1, myTemplateParam val2) {
      aTemplateVal = val1 + val2;
    }

  private:
    myTemplateParam aTemplateVal;
  };

  // DIVA Object: Class.
  class MyDivaObjectClass {
  public:
    // DIVA Object: Member.
    MyDivaObjectClass() {}
  };

  struct MyDivaObjectStruct {
  public:
    // DIVA Object: Member.
    MyDivaObjectStruct() {}
  };

  // DIVA Object: Union.
  union MyUnion {
    int m_int;
    char m_char;
  };

  // DIVA Object: Function, Parameter, Block.
  int myFunc1(character aParam) {
    return 1;
  }
}

// DIVA Object: Using.
using namespace AllObjects;
using AllObjects::MyEnum;

// DIVA Object: Function.
int main() {

  // DIVA Object: CodeLine, PrimitiveType.
  int two = myFunc1('a') + myFunc1('a');

  // DIVA Object: Codeline, Type.
  MyDivaObjectTemplate<int> four(2, 2);

  MyUnion aUnionThing;

  MyDivaObjectStruct aStructThing;

  MyEnum anEnum = zero;

  // DIVA Object: Block.
  {
    int a = 10;
    int myLocalResult = a + a;
  }

  // DIVA Object: CodeLine.
  return 0;
}
