public class TestException {
    public static int EntryPoint() {
        try {
            new TestException().throwingMethod();
            return 2;
        } catch (Throwable throwable) {
            return 1;
        }
    }

    private void throwingMethod() throws Throwable {
        throw new Throwable();
    }
}
