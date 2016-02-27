int g(int x)
{
    return x+2;
}

int f(int x)
{
    return g(x);
}

int main(void)
{
    return f(3)+1;
}
