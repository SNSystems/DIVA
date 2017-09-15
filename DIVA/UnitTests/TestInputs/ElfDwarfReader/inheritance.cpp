class Base {

};

class Public : public Base {

};

class Private : private Base {

};

class Protected : protected Base {

};

class Default : Base {

};

void func() {
    Base base;
    Public pub;
    Private priv;
    Protected prot;
    Default def;
}
