package hw4;

public class HungryBirds {
    public static void main(String[] args) {
        Dish dish = new Dish(10, 10);
        Thread[] babyBirds = new Thread[5];
        Thread parentBird = new Thread(new ParentBird(dish));
        parentBird.start();
        for (int i = 0; i < babyBirds.length; i++) {
            babyBirds[i] = new Thread(new BabyBird(i, dish));
            babyBirds[i].start();
        }
    }
}
