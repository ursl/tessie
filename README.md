# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)

## Installation
>[!IMPORTANT]
>Please start with the [user guide](https://github.com/ursl/tessie/blob/master/main.pdf).
>The installation procedure for `tessie` is described in (gory) detail.

You could also look at [Bane's scripts](https://github.com/BranislavRistic/tessie/tree/dev) for quasi-automatic installation (it may or may not work out of the box, depending which OS version you start with).

## Documentation
All documentation that used to be on this readme page has been moved to the [user guide](https://github.com/ursl/tessie/blob/master/main.pdf).

## Tags/Releases
Periodically, the tessie code base is tagged to provide fixed versions. We use the date and a numerical suffix for tagging. The following table provides an overview of releases/tag. You can either download the linked tar/zip files or checkout as described in the [user guide](https://github.com/ursl/tessie/blob/master/main.pdf).



|Date   | Tag  | Links   | Remarks   |
| ----- | ----------- | ----- | ---- |
| 2025/05/14 | [2025/05/14-02](https://github.com/ursl/tessie/releases/tag/2025%2F05%2F14-02) &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2025/05/14-02.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2025/05/14-02.tar.gz) | fix issue #37: provide 'get {monitoring|allMonTessie}' |
| 2025/04/29 | [2025/04/28-02](https://github.com/ursl/tessie/releases/tag/2025%2F04%2F29-01) &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2025/04/29-01.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2025/04/29-01.tar.gz) | Apply offset correction to HYT-223 RH reading. Show tessieWeb version. |
| 2025/04/28 | [2025/04/28-02](https://github.com/ursl/tessie/releases/tag/2025%2F04%2F28-02) &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2025/04/28-02.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2025/04/28-02.tar.gz) | require 10 sequential readings of chiller before shutdown. Fix to issue #38. |
| 2025/01/30 | [2025/01/30-01](https://github.com/ursl/tessie/releases/tag/2025%2F01%2F30-01) &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2025/01/30-01.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2025/01/30-01.tar.gz) | TEC f/w version should be properly displayed on tessieWeb |
| 2024/10/25 | [2024/10/25-02](https://github.com/ursl/tessie/releases/tag/2024%2F10%2F25-02) &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2024/10/25-02.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2024/10/25-02.tar.gz) | closes issue #28 and add flowswitch status to touchscreen GUI |
| 2024/10/10 | [2024/10/10-01](https://github.com/ursl/tessie/releases/tag/2024%2F10%2F10-01)  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2024/10/10-01.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2024/10/10-01.tar.gz) | addresses issue #26 (but you also need to modify `/lib/systemd/system/tessie.service` as described in the user guide) |
|2024/09/03 |  [2024/09/03-01](https://github.com/ursl/tessie/releases/tag/2024%2F09%2F03-01)  | [zip](https://github.com/ursl/tessie/archive/refs/tags/2024/09/03-01.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2024/09/03-01.tar.gz) | Add Load and Save buttons to web UI for loading/saving TEC parameters into flash  |
|2024/08/28 |  [2024/08/28-03](https://github.com/ursl/tessie/releases/tag/2024%2F08%2F28-03)   | [zip](https://github.com/ursl/tessie/archive/refs/tags/2024/08/28-03.zip) [tar](https://github.com/ursl/tessie/archive/refs/tags/2024/08/28-03.tar.gz) | tessie will shutdown operations if unsafe conditions detected  |
