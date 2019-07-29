/*
 *
 * Copyright 2018 NEM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

const nem2Sdk = require("nem2-sdk");
const Account = nem2Sdk.Account,
    Deadline = nem2Sdk.Deadline,
    NetworkType = nem2Sdk.NetworkType,
    TransferTransaction = nem2Sdk.TransferTransaction,
    AggregateTransaction = nem2Sdk.AggregateTransaction,
    TransactionHttp = nem2Sdk.TransactionHttp,
    PlainMessage = nem2Sdk.PlainMessage,
    Address = nem2Sdk.Address,
    Mosaic = nem2Sdk.Mosaic,
    UInt64 = nem2Sdk.UInt64,
    MosaicId = nem2Sdk.MosaicId;

/* start block 01 */
const transactionHttp = new TransactionHttp('http://52.194.207.217:3000');

const privateKey = "75938334DAFA7EF743FB2A9694C8025648959DC8FDE61678AC99281826BED7A3";
const account = Account.createFromPrivateKey(privateKey, NetworkType.MIJIN_TEST);

const brotherAddress = 'SC7APJ3C6BWK3DWVMNUJRQXETYMIC6Y2OG557QHU';
const brotherAccount = Address.createFromRawAddress(brotherAddress);

const sisterAddress = 'SCQVB7TIAWI7OJFGRRRU57UPJ4BFNHORCE2JTV2J';
const sisterAccount = Address.createFromRawAddress(sisterAddress);

mosaicId = "77a1969932d987d7";  
mosaicamount = 10;
const amount = [new Mosaic(new MosaicId(mosaicId), UInt64.fromUint(mosaicamount))]; // 10 cat.currency represent 10 000 000 micro cat.currency

const brotherTransferTransaction = TransferTransaction.create(Deadline.create(), brotherAccount, amount, PlainMessage.create('payout'), NetworkType.MIJIN_TEST);
const sisterTransferTransaction = TransferTransaction.create(Deadline.create(), sisterAccount, amount, PlainMessage.create('payout'), NetworkType.MIJIN_TEST);

const aggregateTransaction = AggregateTransaction.createComplete(
    Deadline.create(),
    [brotherTransferTransaction.toAggregate(account.publicAccount),
        sisterTransferTransaction.toAggregate(account.publicAccount)],
    NetworkType.MIJIN_TEST,
    []
);
/* end block 01 */

/* start block 02 */
const networkGenerationHash = "9F1979BEBA29C47E59B40393ABB516801A353CFC0C18BC241FEDE41939C907E7";
const signedTransaction = account.sign(aggregateTransaction, networkGenerationHash);
console.log(signedTransaction.hash);

transactionHttp
    .announce(signedTransaction)
    .subscribe(x => console.log(x), err => console.error(err));
/* end block 02 */