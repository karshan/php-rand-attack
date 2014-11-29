<?php
print("pid: " . getmypid() . "\n");
print("time: " . time() . "\n");
for ($i = 0; $i < 10; $i++) {
    print(rand() . "\n");
}
?>
