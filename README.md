# YadroTest

## Замечания по решению
* Клиент не может встать в очередь, не прийдя (событие "ID 1 Клиент пришел" обязательно до "ID 3 Клиент ожидает" - иначе "ClientUnknown")
* Если клиент пересаживается, то заканчивается предудущая сессия и происходит расчёт, потом начинается новая сессия за новым столом. Иначе не понятно - как разделить выручку по столам, за которыми сидел клиент.
* Тесты в процессе написания
