import sys
import random
import string

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <test directory>")
        sys.exit(1)

    test_dir = sys.argv[1]

    line_count = [1, 5, 10, 20, 5]
    for test_count in range(5):
        name_len = random.randint(1, 5)
        name = "file_" + "".join([random.choice(string.ascii_letters) for _ in range(name_len)])

        test = []

        for _ in range(line_count[test_count]):
            val_list = [round(random.uniform(-5.0, 15.0), random.randint(0, 5)) for _ in range(3)]
            test.append(val_list)

        # сгенерируем случай с нулем
        if test_count == 4:
            test[2][1] = 0.0

        test_name = "{}/test_{}".format(test_dir, test_count + 1)
        with open(f'{test_name}.txt', 'w') as ftest:
            ftest.write(f'{name}\n')
            for t in test:
                ftest.write(f'{t[0]} {t[1]} {t[2]}\n')

main()
